#pragma once

#include <JuceHeader.h>

#if JUCE_WINDOWS

// A virtual output-only device which exposes every shared-mode WASAPI endpoint
// as a stereo pair. One endpoint is the render clock; the others consume from
// bounded FIFOs and very gently add/drop one source frame to follow that clock.
class AggregatedWASAPIAudioDevice final : public juce::AudioIODevice
{
    struct Endpoint;
public:
    explicit AggregatedWASAPIAudioDevice(juce::StringArray endpointNames)
        : juce::AudioIODevice("All WASAPI Outputs", "Aggregated WASAPI"),
          endpointNames(std::move(endpointNames))
    {
        for (const auto& endpoint : this->endpointNames)
        {
            outputNames.add(endpoint + " (L)");
            outputNames.add(endpoint + " (R)");
        }
    }

    ~AggregatedWASAPIAudioDevice() override { close(); }

    juce::StringArray getOutputChannelNames() override { return outputNames; }
    juce::StringArray getInputChannelNames() override { return {}; }
    std::optional<juce::BigInteger> getDefaultOutputChannels() const override
    {
        juce::BigInteger result;
        // JUCE must see the complete virtual bus for the lifetime of this device.
        // Physical endpoints are opened independently by our left-hand mask.
        if (!outputNames.isEmpty()) result.setRange(0, outputNames.size(), true);
        return result;
    }
    std::optional<juce::BigInteger> getDefaultInputChannels() const override { return juce::BigInteger(); }
    juce::Array<double> getAvailableSampleRates() override
    {
        return { 44100.0, 48000.0, 88200.0, 96000.0 };
    }
    juce::Array<int> getAvailableBufferSizes() override
    {
        return { 64, 128, 192, 256, 384, 480, 512, 768, 1024, 2048 };
    }
    // The endpoints are shared-mode streams fed by the Windows audio engine in
    // 10 ms packets (480 samples at 48 kHz): a smaller buffer only splits the
    // packet into shorter deadlines (an IR's tail FFT no longer fits a 256
    // block) without lowering the latency. 480 is the natural size.
    int getDefaultBufferSize() override { return 480; }

    juce::String open(const juce::BigInteger&, const juce::BigInteger& requestedOutputs,
        double requestedRate, int requestedBufferSize) override
    {
        close();
        // The callback always receives the complete virtual bus (one pointer per
        // output name), so the active-channel report must match it exactly.
        // juce::AudioDeviceManager sizes its pointer arrays from this report;
        // reporting fewer channels than delivered corrupted the heap.
        juce::ignoreUnused(requestedOutputs);
        hostOutputs.clear();
        if (!outputNames.isEmpty()) hostOutputs.setRange(0, outputNames.size(), true);
        // No physical endpoint yet: the route's configured channels are applied by
        // its callback the moment the device starts (setSelectedOutputChannels),
        // on every path - loading, "set for all", a type switch, a reopen. Until
        // 0.8.47 the first endpoint of the list was opened here and stayed the
        // only one on any path that forgot to apply the channels (the auxiliary
        // routes on loading): every route then played through the same two
        // channels whatever was set, and a dead first endpoint (an unconnected
        // S/PDIF) failed the whole device.
        activeOutputs.clear();
        updateActiveFlags(activeOutputs);
        sampleRate = requestedRate > 0.0 ? requestedRate : 48000.0;
        bufferSize = requestedBufferSize > 0 ? requestedBufferSize : 256;
        lastError.clear();
        endpointErrors.clear();
        opened = true;
        return {};
    }

    // Changes the physical endpoint set without deleting this AudioIODevice.
    // AudioDeviceSelectorComponent owns UI nodes tied to the outer device, so
    // recreating that device from inside a checkbox callback is not safe.
    // Endpoints that cannot be opened (unplugged, no cable on a digital output)
    // are refused: their channels are left out of the selection, exactly as the
    // Windows Audio type refuses such a device, and named in the returned text.
    // The selection in force afterwards is getSelectedOutputChannels().
    juce::String setSelectedOutputChannels(const juce::BigInteger& requestedOutputs)
    {
        if (requestedOutputs == activeOutputs) return {};
        juce::String refused;
        const bool restart = playing;
        if (restart)
        {
            for (auto* endpoint : endpoints)
                if (endpoint->device) endpoint->device->stop();
            waitForEndpointCallbacks();
            for (auto* endpoint : endpoints)
                if (endpoint->device
                    && !requestedOutputs[endpoint->index * 2]
                    && !requestedOutputs[endpoint->index * 2 + 1])
                    endpoint->device->close();
        }
        juce::BigInteger granted = requestedOutputs;   // minus the endpoints that cannot be opened
        {
            const juce::ScopedLock lock(renderLock);
            for (int endpointIndex = 0; endpointIndex < endpointNames.size(); ++endpointIndex)
            {
                if (!requestedOutputs[endpointIndex * 2]
                    && !requestedOutputs[endpointIndex * 2 + 1]) continue;
                Endpoint* existing = nullptr;
                for (auto* endpoint : endpoints)
                    if (endpoint->index == endpointIndex) existing = endpoint;
                bool ready = false;
                if (existing == nullptr) ready = openEndpoint(endpointIndex) != nullptr;
                else if (existing->device && existing->device->isOpen()) ready = true;
                else if (existing->device)
                {
                    juce::BigInteger physicalOutputs;
                    physicalOutputs.setRange(0, juce::jmin(2,
                        existing->device->getOutputChannelNames().size()), true);
                    const auto error = existing->device->open({}, physicalOutputs,
                        sampleRate, bufferSize);
                    ready = error.isEmpty();
                    if (!ready) setEndpointError(endpointIndex, error.isEmpty() ? juce::String("cannot be reopened") : error);
                }
                if (!ready)
                {
                    granted.clearBit(endpointIndex * 2);
                    granted.clearBit(endpointIndex * 2 + 1);
                    refused << (refused.isEmpty() ? "" : "; ") << "\"" << endpointNames[endpointIndex] << "\" "
                        << endpointError(endpointIndex);
                }
            }
            activeOutputs = granted;
            updateActiveFlags(granted);
            master = nullptr;
            for (auto* endpoint : endpoints)
                if (granted[endpoint->index * 2]
                    || granted[endpoint->index * 2 + 1])
                {
                    master = endpoint;
                    break;
                }
        }
        // Endpoints are deliberately retained until the aggregate itself closes.
        // Removing an endpoint while a WASAPI worker is returning from its callback
        // caused the observed list/memset crashes. Deselection merely silences it.
        if (restart && master != nullptr)
        {
            for (auto* endpoint : endpoints)
                if (granted[endpoint->index * 2]
                    || granted[endpoint->index * 2 + 1])
                    endpoint->prepareForStart();
            // Start consumers before the producer. Starting the master first
            // allowed it to write while a slave's aboutToStart reset its FIFO.
            for (auto* endpoint : endpoints)
                if (endpoint != master && (granted[endpoint->index * 2]
                    || granted[endpoint->index * 2 + 1]))
                    endpoint->device->start(endpoint);
            master->device->start(master);
        }
        return refused;
    }

    juce::BigInteger getSelectedOutputChannels() const { return activeOutputs; }

    // Why an endpoint could not be opened the last time it was asked for ("" when it opened).
    juce::String endpointError(int endpointIndex) const
    {
        return endpointIndex >= 0 && endpointIndex < endpointErrors.size() ? endpointErrors[endpointIndex] : juce::String();
    }
    juce::String endpointErrorForChannel(int channel) const { return endpointError(channel / 2); }

    // The endpoints that play now, for the activity log: "A (L+R), B (L)" or "no endpoint".
    juce::String selectionDescription() const
    {
        juce::String text;
        for (int endpointIndex = 0; endpointIndex < endpointNames.size(); ++endpointIndex)
        {
            const bool left = activeOutputs[endpointIndex * 2], right = activeOutputs[endpointIndex * 2 + 1];
            if (!left && !right) continue;
            text << (text.isEmpty() ? "" : ", ") << "\"" << endpointNames[endpointIndex] << "\" ("
                 << (left && right ? "L+R" : (left ? "L" : "R")) << ")";
        }
        return text.isEmpty() ? juce::String("no endpoint") : text;
    }

private:
    void setEndpointError(int endpointIndex, const juce::String& error)
    {
        while (endpointErrors.size() <= endpointIndex) endpointErrors.add({});
        endpointErrors.set(endpointIndex, error);
    }

    Endpoint* openEndpoint(int endpointIndex);

    void updateActiveFlags(const juce::BigInteger& mask)
    {
        for (int channel = 0; channel < (int)activeFlags.size(); ++channel)
            activeFlags[(size_t)channel].store(mask[channel], std::memory_order_release);
    }

    bool isChannelActive(int channel) const noexcept
    {
        return channel >= 0 && channel < (int)activeFlags.size()
            && activeFlags[(size_t)channel].load(std::memory_order_acquire);
    }

public:

    void close() override
    {
        stop();
        waitForEndpointCallbacks();
        for (auto* endpoint : endpoints)
            if (endpoint->device) endpoint->device->close();
        endpoints.clear();
        sharedWasapiType.reset();
        opened = false;
    }

    bool isOpen() override { return opened; }

    void start(juce::AudioIODeviceCallback* newCallback) override
    {
        if (!opened || newCallback == nullptr) return;
        callback = newCallback;
        callback->audioDeviceAboutToStart(this);
        master = nullptr;
        for (auto* endpoint : endpoints)
            if (isChannelActive(endpoint->index * 2)
                || isChannelActive(endpoint->index * 2 + 1))
            {
                master = endpoint;
                break;
            }
        if (master == nullptr)
        {
            playing = true;
            return;
        }
        for (auto* endpoint : endpoints)
            if (isChannelActive(endpoint->index * 2)
                || isChannelActive(endpoint->index * 2 + 1))
                endpoint->prepareForStart();
        // Consumers start empty and silent; start the producer last so no FIFO
        // can be reset concurrently with its first write.
        for (auto* endpoint : endpoints)
            if (endpoint != master
                && (isChannelActive(endpoint->index * 2)
                    || isChannelActive(endpoint->index * 2 + 1)))
                endpoint->device->start(endpoint);
        master->device->start(master);
        playing = true;
    }

    void stop() override
    {
        if (!playing && callback == nullptr) return;
        for (auto* endpoint : endpoints)
            if (endpoint->device) endpoint->device->stop();
        if (callback) callback->audioDeviceStopped();
        callback = nullptr;
        master = nullptr;
        playing = false;
    }

    bool isPlaying() override { return playing; }
    juce::String getLastError() override { return lastError; }
    int getCurrentBufferSizeSamples() override { return bufferSize; }
    double getCurrentSampleRate() override { return sampleRate; }
    int getCurrentBitDepth() override { return 32; }
    juce::BigInteger getActiveOutputChannels() const override { return hostOutputs; }
    juce::BigInteger getActiveInputChannels() const override { return {}; }
    int getOutputLatencyInSamples() override
    {
        int result = bufferSize;
        for (auto* endpoint : endpoints)
            result = juce::jmax(result, endpoint->device->getOutputLatencyInSamples());
        return result + bufferSize * 3;
    }
    int getInputLatencyInSamples() override { return 0; }
    int getXRunCount() const noexcept override { return xruns.load(); }

private:
    static constexpr int fifoCapacity = 262144;

    struct Endpoint final : public juce::AudioIODeviceCallback
    {
        Endpoint(AggregatedWASAPIAudioDevice& owner, int index)
            : owner(owner), index(index), fifo(fifoCapacity), fifoAudio(2, fifoCapacity) {}

        void prepareForStart()
        {
            fifo.reset();
            fifoAudio.clear();
            transfer.clear();
            primed = false;
        }
        void audioDeviceAboutToStart(juce::AudioIODevice*) override {}
        void audioDeviceStopped() override { primed = false; }
        void audioDeviceError(const juce::String& error) override
        {
            owner.lastError = endpointName() + ": " + error;
            owner.xruns.fetch_add(1);
        }
        void audioDeviceIOCallbackWithContext(const float* const*, int,
            float* const* outputs, int numOutputs, int numSamples,
            const juce::AudioIODeviceCallbackContext& context) override
        {
            const juce::ScopedLock callbackGuard(callbackLock);
            for (int channel = 0; channel < numOutputs; ++channel)
                if (outputs[channel]) juce::FloatVectorOperations::clear(outputs[channel], numSamples);
            if (this == owner.master) owner.renderMaster(*this, outputs, numOutputs, numSamples, context);
            else owner.renderSlave(*this, outputs, numOutputs, numSamples);
        }

        juce::String endpointName() const { return owner.endpointNames[index]; }
        AggregatedWASAPIAudioDevice& owner;
        int index;
        std::unique_ptr<juce::AudioIODevice> device;
        juce::AbstractFifo fifo;
        juce::AudioBuffer<float> fifoAudio;
        juce::AudioBuffer<float> transfer;
        bool primed = false;
        juce::CriticalSection callbackLock;
    };

    void waitForEndpointCallbacks()
    {
        // Taking every callback lock after AudioIODevice::stop() establishes
        // that no WASAPI worker still references buffers owned by an endpoint.
        for (auto* endpoint : endpoints)
        {
            const juce::ScopedLock finished(endpoint->callbackLock);
        }
    }

    void renderMaster(Endpoint& reference, float* const* outputs, int numOutputs,
        int numSamples, const juce::AudioIODeviceCallbackContext& context)
    {
        const juce::ScopedLock lock(renderLock);
        virtualBuffer.setSize(outputNames.size(), numSamples, false, false, true);
        virtualBuffer.clear();
        outputPointers.resize((size_t)outputNames.size());
        // Every channel gets a real buffer. juce::AudioDeviceManager assumes
        // non-null output pointers (it memsets them when it has no callbacks),
        // so a nullptr here crashed the moment a route was moved to ASIO while
        // this aggregate was still running. Inactive channels are simply never
        // copied to a physical endpoint.
        for (int channel = 0; channel < outputNames.size(); ++channel)
            outputPointers[(size_t)channel] = virtualBuffer.getWritePointer(channel);
        if (callback)
            callback->audioDeviceIOCallbackWithContext(nullptr, 0, outputPointers.data(),
                outputNames.size(), numSamples, context);

        copyVirtualPair(reference, outputs, numOutputs, numSamples);
        for (auto* endpoint : endpoints)
            if (endpoint != &reference) pushToFifo(*endpoint, numSamples);
    }

    void copyVirtualPair(Endpoint& endpoint, float* const* outputs, int numOutputs, int numSamples)
    {
        for (int channel = 0; channel < juce::jmin(2, numOutputs); ++channel)
            if (outputs[channel] != nullptr)
            {
                const int global = endpoint.index * 2 + channel;
                if (isChannelActive(global))
                    juce::FloatVectorOperations::copy(outputs[channel],
                        virtualBuffer.getReadPointer(global), numSamples);
            }
    }

    void pushToFifo(Endpoint& endpoint, int numSamples)
    {
        const int free = endpoint.fifo.getFreeSpace();
        if (free < numSamples)
        {
            xruns.fetch_add(1);
            return; // only the consumer may advance the FIFO read position
        }
        const auto scope = endpoint.fifo.write(numSamples);
        int sourceOffset = 0;
        const juce::Range<int> blocks[] {
            { scope.startIndex1, scope.startIndex1 + scope.blockSize1 },
            { scope.startIndex2, scope.startIndex2 + scope.blockSize2 }
        };
        for (const auto& block : blocks)
        {
            for (int channel = 0; channel < 2; ++channel)
                endpoint.fifoAudio.copyFrom(channel, block.getStart(), virtualBuffer,
                    endpoint.index * 2 + channel, sourceOffset, block.getLength());
            sourceOffset += block.getLength();
        }
    }

    void renderSlave(Endpoint& endpoint, float* const* outputs, int numOutputs, int numSamples)
    {
        const int target = juce::jmax(bufferSize * 3, numSamples * 2);
        int ready = endpoint.fifo.getNumReady();
        if (!endpoint.primed)
        {
            if (ready < target) return;
            endpoint.primed = true;
        }
        if (ready < juce::jmax(2, numSamples - 1))
        {
            endpoint.primed = false;
            xruns.fetch_add(1);
            return;
        }

        const int correction = ready > target + bufferSize ? 1
            : (ready < target - bufferSize && numSamples > 2 ? -1 : 0);
        const int sourceFrames = juce::jlimit(2, numSamples + 1, numSamples + correction);
        endpoint.transfer.setSize(2, sourceFrames, false, false, true);
        const auto scope = endpoint.fifo.read(sourceFrames);
        int destinationOffset = 0;
        const juce::Range<int> blocks[] {
            { scope.startIndex1, scope.startIndex1 + scope.blockSize1 },
            { scope.startIndex2, scope.startIndex2 + scope.blockSize2 }
        };
        for (const auto& block : blocks)
        {
            for (int channel = 0; channel < 2; ++channel)
                endpoint.transfer.copyFrom(channel, destinationOffset, endpoint.fifoAudio,
                    channel, block.getStart(), block.getLength());
            destinationOffset += block.getLength();
        }

        for (int channel = 0; channel < juce::jmin(2, numOutputs); ++channel)
        {
            if (outputs[channel] == nullptr || !isChannelActive(endpoint.index * 2 + channel)) continue;
            if (sourceFrames == numSamples)
                juce::FloatVectorOperations::copy(outputs[channel],
                    endpoint.transfer.getReadPointer(channel), numSamples);
            else
            {
                const float* source = endpoint.transfer.getReadPointer(channel);
                for (int i = 0; i < numSamples; ++i)
                {
                    const double position = (double)i * (sourceFrames - 1)
                        / juce::jmax(1, numSamples - 1);
                    const int a = juce::jlimit(0, sourceFrames - 1, (int)position);
                    const int b = juce::jmin(sourceFrames - 1, a + 1);
                    const float alpha = (float)(position - a);
                    outputs[channel][i] = source[a] + alpha * (source[b] - source[a]);
                }
            }
        }
    }

    juce::StringArray endpointNames, outputNames;
    juce::OwnedArray<Endpoint> endpoints;
    std::unique_ptr<juce::AudioIODeviceType> sharedWasapiType;   // outlives the endpoints
    Endpoint* master = nullptr;
    juce::AudioIODeviceCallback* callback = nullptr;
    juce::AudioBuffer<float> virtualBuffer;
    std::vector<float*> outputPointers;
    juce::BigInteger activeOutputs, hostOutputs;
    juce::StringArray endpointErrors;   // per endpoint index: why it could not be opened ("" = opened)
    std::array<std::atomic_bool, 512> activeFlags{};
    juce::CriticalSection renderLock;
    std::atomic<int> xruns{ 0 };
    juce::String lastError;
    double sampleRate = 48000.0;
    int bufferSize = 256;
    bool opened = false, playing = false;
};

inline AggregatedWASAPIAudioDevice::Endpoint*
AggregatedWASAPIAudioDevice::openEndpoint(int endpointIndex)
{
    auto endpoint = std::make_unique<Endpoint>(*this, endpointIndex);
    // One WASAPI device type (and one endpoint scan) per aggregate; scanning
    // once per endpoint made opening the aggregate take seconds.
    if (sharedWasapiType == nullptr)
    {
        sharedWasapiType.reset(juce::AudioIODeviceType::createAudioIODeviceType_WASAPI(
            juce::WASAPIDeviceMode::shared));
        if (sharedWasapiType == nullptr)
        {
            lastError = "Shared-mode WASAPI is unavailable.";
            setEndpointError(endpointIndex, lastError);
            return nullptr;
        }
        sharedWasapiType->scanForDevices();
    }
    endpoint->device.reset(sharedWasapiType->createDevice(endpointNames[endpointIndex], {}));
    if (!endpoint->device) { setEndpointError(endpointIndex, "is not present"); return nullptr; }
    juce::BigInteger physicalOutputs;
    const int physicalCount = juce::jmin(2, endpoint->device->getOutputChannelNames().size());
    if (physicalCount <= 0) { setEndpointError(endpointIndex, "has no output channel"); return nullptr; }
    physicalOutputs.setRange(0, physicalCount, true);
    const auto error = endpoint->device->open({}, physicalOutputs, sampleRate, bufferSize);
    if (error.isNotEmpty())
    {
        DBG("Aggregated WASAPI: skipping " + endpointNames[endpointIndex] + ": " + error);
        setEndpointError(endpointIndex, "cannot be opened: " + error);
        return nullptr;
    }
    setEndpointError(endpointIndex, {});
    return endpoints.add(std::move(endpoint));
}

class AggregatedWASAPIAudioDeviceType final : public juce::AudioIODeviceType
{
public:
    AggregatedWASAPIAudioDeviceType() : juce::AudioIODeviceType("Aggregated WASAPI") {}
    void scanForDevices() override
    {
        auto type = std::unique_ptr<juce::AudioIODeviceType>(
            juce::AudioIODeviceType::createAudioIODeviceType_WASAPI(juce::WASAPIDeviceMode::shared));
        endpoints.clear();
        if (type)
        {
            type->scanForDevices();
            endpoints = type->getDeviceNames(false);
        }
    }
    juce::StringArray getDeviceNames(bool wantInputNames = false) const override
    {
        return wantInputNames || endpoints.isEmpty() ? juce::StringArray{}
            : juce::StringArray{ "All WASAPI Outputs" };
    }
    int getDefaultDeviceIndex(bool forInput) const override { return forInput ? -1 : 0; }
    int getIndexOfDevice(juce::AudioIODevice* device, bool asInput) const override
    {
        return !asInput && device != nullptr && device->getName() == "All WASAPI Outputs" ? 0 : -1;
    }
    bool hasSeparateInputsAndOutputs() const override { return true; }
    juce::AudioIODevice* createDevice(const juce::String& outputName,
        const juce::String&) override
    {
        return outputName == "All WASAPI Outputs"
            ? new AggregatedWASAPIAudioDevice(endpoints) : nullptr;
    }
private:
    juce::StringArray endpoints;
};

inline void addAggregatedWASAPIType(juce::AudioDeviceManager& manager)
{
    auto type = std::make_unique<AggregatedWASAPIAudioDeviceType>();
    // AudioDeviceManager may already have completed its one lazy scan before a
    // custom type is appended. Scan explicitly so the first selection has names.
    type->scanForDevices();
    manager.addAudioDeviceType(std::move(type));
}

#else
// Other platforms have no aggregate: a stub so the dynamic_casts in
// MainComponent compile (they always yield nullptr there).
class AggregatedWASAPIAudioDevice final : public juce::AudioIODevice
{
public:
    AggregatedWASAPIAudioDevice() : juce::AudioIODevice("All WASAPI Outputs", "Aggregated WASAPI") {}
    juce::StringArray getOutputChannelNames() override { return {}; }
    juce::StringArray getInputChannelNames() override { return {}; }
    juce::Array<double> getAvailableSampleRates() override { return {}; }
    juce::Array<int> getAvailableBufferSizes() override { return {}; }
    int getDefaultBufferSize() override { return 256; }
    juce::String open(const juce::BigInteger&, const juce::BigInteger&, double, int) override { return "unavailable"; }
    void close() override {}
    bool isOpen() override { return false; }
    void start(juce::AudioIODeviceCallback*) override {}
    void stop() override {}
    bool isPlaying() override { return false; }
    juce::String getLastError() override { return {}; }
    int getCurrentBufferSizeSamples() override { return 0; }
    double getCurrentSampleRate() override { return 0.0; }
    int getCurrentBitDepth() override { return 0; }
    juce::BigInteger getActiveOutputChannels() const override { return {}; }
    juce::BigInteger getActiveInputChannels() const override { return {}; }
    int getOutputLatencyInSamples() override { return 0; }
    int getInputLatencyInSamples() override { return 0; }
    juce::String setSelectedOutputChannels(const juce::BigInteger&) { return {}; }
    juce::BigInteger getSelectedOutputChannels() const { return {}; }
    juce::String endpointErrorForChannel(int) const { return {}; }
    juce::String selectionDescription() const { return {}; }
};
inline void addAggregatedWASAPIType(juce::AudioDeviceManager&) {}
#endif
