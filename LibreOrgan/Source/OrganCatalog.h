#pragma once

#include <JuceHeader.h>
#include "MainComponent.h"
#include "UIControls.h"
#include "QuietProcess.h"
#include "CrashHandler.h"

// 0.9.145 THE ORGAN CATALOG (the user: "Load From Internet Catalog ... it would download itself onto his computer
// and load"): the configurations published in the repository's libreorgans/ folder, listed in the program.
//
// Nothing new behind it - one more way INTO the existing paths:
//   - the list is libreorgans/catalog.json, written by the robot that writes INDEX.md (configurations_index.py)
//     from the same published zips: the columns of the website's table (organ, divisions, stops, RAM, routes),
//     the zip's link, the folder trees the organ expects with their fingerprints;
//   - a chosen organ's zip is downloaded and its configuration handed to MainComponent::loadConfigurationFile -
//     the load box, the copy into the LibreOrgans folder, keep / replace, the merge: all as with Load Configuration;
//   - "samples on this computer" asks the folder search a load uses (MainComponent::catalogTreesHere), quietly;
//   - an organ whose samples are free (libreorgans/SAMPLES.txt: the Rural Organ demo) can be fetched with them:
//     the archive goes to Documents/LibreOrgan/LibreSamples and is loaded as Load Configuration loads a .rar.
// The internet is asked only when the user opens this window or presses a button in it - never by itself.
// A configuration from the catalog is opened with its plug-ins switched OFF (they are another computer's choice).
namespace OrganCatalog
{
    inline juce::String catalogLink()
    {
        // (ORGAN_CATALOG_URL: a link or a file on this computer - for tests and for a catalog kept by hand)
        if (const char* env = std::getenv("ORGAN_CATALOG_URL")) if (*env != 0) return juce::String::fromUTF8(env);
        return "https://raw.githubusercontent.com/zurek-jiri/LibreOrgan/main/libreorgans/catalog.json";
    }

    struct Entry
    {
        juce::String title, folder, zip, url, readme, routes, producer, link, rootName, rootRelative;
        juce::StringArray trees;
        juce::StringPairArray fingerprints;
        int divisions = 0, stops = 0;
        juce::int64 ramBytes = 0, zipBytes = 0;
        bool featured = false;
        juce::String samplesUrl, samplesName;   // free sample folders in one archive (SAMPLES.txt), else empty
        juce::int64 samplesBytes = 0;
        int treesHere = -1;                     // -1 = not looked yet
    };

    inline std::vector<Entry> entriesOf(const juce::var& catalog)
    {
        std::vector<Entry> result;
        const auto* list = catalog.getProperty("organs", juce::var()).getArray();
        if (list == nullptr) return result;
        for (const auto& item : *list)
        {
            if (!item.isObject()) continue;
            Entry e;
            e.title = item.getProperty("title", "").toString();
            e.folder = item.getProperty("folder", "").toString();
            e.zip = item.getProperty("zip", "").toString();
            e.url = item.getProperty("url", "").toString();
            e.readme = item.getProperty("readme", "").toString();
            e.producer = item.getProperty("producer", "").toString();
            e.link = item.getProperty("link", "").toString();
            e.rootName = item.getProperty("rootName", "").toString();
            e.rootRelative = item.getProperty("rootRelative", "").toString();
            e.divisions = (int)item.getProperty("divisions", 0);
            e.stops = (int)item.getProperty("stops", 0);
            e.ramBytes = (juce::int64)item.getProperty("ramBytes", 0);
            e.zipBytes = (juce::int64)item.getProperty("zipBytes", 0);
            e.featured = (bool)item.getProperty("featured", false);
            juce::StringArray routes;
            if (const auto* r = item.getProperty("routes", juce::var()).getArray()) for (const auto& name : *r) routes.add(name.toString());
            e.routes = routes.joinIntoString(", ");
            if (const auto* t = item.getProperty("trees", juce::var()).getArray()) for (const auto& tree : *t) e.trees.add(tree.toString());
            if (const auto* prints = item.getProperty("fingerprints", juce::var()).getDynamicObject())
                for (const auto& p : prints->getProperties()) e.fingerprints.set(p.name.toString(), p.value.toString());
            const auto samples = item.getProperty("samples", juce::var());
            if (samples.isObject())
            {
                e.samplesUrl = samples.getProperty("url", "").toString();
                e.samplesName = juce::File::createLegalFileName(samples.getProperty("name", "").toString());
                e.samplesBytes = (juce::int64)samples.getProperty("bytes", 0);
            }
            // Only what can be fetched from the repository itself and named safely.
            if (e.title.isEmpty() || e.folder.isEmpty() || !e.url.startsWithIgnoreCase("https://")) continue;
            if (e.samplesUrl.isNotEmpty() && (!e.samplesUrl.startsWithIgnoreCase("https://") || e.samplesName.isEmpty())) { e.samplesUrl.clear(); e.samplesName.clear(); }
            result.push_back(std::move(e));
        }
        return result;
    }

    inline juce::String sizeText(juce::int64 bytes)
    {
        if (bytes <= 0) return "-";
        const double gb = (double)bytes / (double)(1 << 30);
        if (gb >= 10.0) return juce::String(juce::roundToInt(gb)) + " GB";
        if (gb >= 1.0) return juce::String(gb, 1) + " GB";
        if (bytes < (1 << 20)) return juce::String(juce::jmax(1, juce::roundToInt((double)bytes / 1024.0))) + " KB";
        return juce::String(juce::roundToInt((double)bytes / (double)(1 << 20))) + " MB";
    }

    // One download - a link, or (tests) a file of this computer - into a file, on a thread of its own. Windows and
    // macOS ask the system's own HTTPS (juce::URL); the Linux build has none of its own (no libcurl is linked: the
    // program depends on nothing that is not part of a plain system), so it runs the system's curl or wget.
    class Download final : private juce::Thread
    {
    public:
        Download(juce::String linkToFetch, juce::File targetFile, juce::int64 expectedBytes)
            : juce::Thread("Catalog download"), link(std::move(linkToFetch)), target(std::move(targetFile)), expected(expectedBytes)
        {
            total.store(expectedBytes);
        }
        ~Download() override { signalThreadShouldExit(); stopThread(15000); }
        void begin() { startThread(); }
        void cancel() { cancelled.store(true); signalThreadShouldExit(); }
        bool running() const { return isThreadRunning(); }
        bool wasCancelled() const { return cancelled.load(); }
        std::atomic<juce::int64> done{ 0 }, total{ 0 };
        // (read once the thread has ended)
        bool ok = false;
        juce::String error;
        const juce::String link;
        const juce::File target;

    private:
        void run() override
        {
            const auto part = target.getSiblingFile(target.getFileName() + ".part");
            target.getParentDirectory().createDirectory();
            part.deleteFile();
            bool fine = false;
            if (juce::File::isAbsolutePath(link))
            {
                const juce::File local(link);
                fine = local.existsAsFile() && local.copyFileTo(part);
                if (!fine) error = "no such file: " + link;
                done.store(part.getSize());
            }
            else
            {
               #if JUCE_LINUX
                fine = fetchWithTool(part);
               #else
                fine = fetchWithSystem(part);
               #endif
            }
            if (fine && !threadShouldExit() && expected > 0 && part.getSize() < expected)
            {
                error = "the download is incomplete (" + sizeText(part.getSize()) + " of " + sizeText(expected) + ")";
                fine = false;
            }
            if (fine && !threadShouldExit())
            {
                target.deleteFile();
                fine = part.moveFileTo(target);
                if (!fine) error = "the file could not be written: " + target.getFullPathName();
            }
            if (!fine || threadShouldExit()) part.deleteFile();
            ok = fine && !threadShouldExit();
        }

       #if JUCE_LINUX
        static juce::String toolOnPath(const juce::String& name)
        {
            for (const auto& dir : juce::StringArray::fromTokens(juce::SystemStats::getEnvironmentVariable("PATH", "/usr/local/bin:/usr/bin:/bin"), ":", ""))
            {
                const auto file = juce::File(dir).getChildFile(name);
                if (dir.startsWithChar('/') && file.existsAsFile()) return file.getFullPathName();
            }
            return {};
        }
        bool fetchWithTool(const juce::File& part)
        {
            juce::StringArray arguments;
            const juce::String curl = toolOnPath("curl");
            const juce::String wget = toolOnPath("wget");
            if (curl.isNotEmpty())
            {
                arguments.add(curl);
                arguments.add("-f"); arguments.add("-L"); arguments.add("-s"); arguments.add("-S");
                arguments.add("--connect-timeout"); arguments.add("20");
                arguments.add("-o"); arguments.add(part.getFullPathName());
                arguments.add(link);
            }
            else if (wget.isNotEmpty())
            {
                arguments.add(wget);
                arguments.add("-q"); arguments.add("--timeout=20");
                arguments.add("-O"); arguments.add(part.getFullPathName());
                arguments.add(link);
            }
            else { error = "neither curl nor wget is installed on this system - one of them is needed for downloads"; return false; }
            QuietProcess process;
            if (!process.start(arguments)) { error = "the download tool could not be started (" + arguments[0] + ")"; return false; }
            while (process.isRunning())
            {
                if (threadShouldExit()) { process.kill(); return false; }
                done.store(part.getSize());
                juce::Thread::sleep(100);
            }
            done.store(part.getSize());
            const int code = process.exitCode();
            if (code != 0) { error = juce::File(arguments[0]).getFileName() + " ended with the code " + juce::String(code) + " (no connection, or the file is not there)"; return false; }
            return part.existsAsFile();
        }
       #else
        bool fetchWithSystem(const juce::File& part)
        {
            int status = 0;
            const auto stream = juce::URL(link).createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                .withConnectionTimeoutMs(20000).withStatusCode(&status).withNumRedirectsToFollow(8));
            if (stream == nullptr) { error = "no connection to " + juce::URL(link).getDomain(); return false; }
            if (status >= 400) { error = "the server answered " + juce::String(status); return false; }
            const auto length = stream->getTotalLength();
            if (length > 0) total.store(length);
            juce::FileOutputStream out(part);
            if (!out.openedOk()) { error = "the file could not be written: " + part.getFullPathName(); return false; }
            juce::HeapBlock<char> buffer(1 << 16);
            juce::int64 written = 0;
            while (!stream->isExhausted())
            {
                if (threadShouldExit()) return false;
                const int got = stream->read(buffer.getData(), 1 << 16);
                if (got < 0) { error = "the connection broke"; return false; }
                if (got == 0) break;
                if (!out.write(buffer.getData(), (size_t)got)) { error = "the disk is full or the file cannot be written"; return false; }
                written += got;
                done.store(written);
            }
            out.flush();
            if (length > 0 && written < length) { error = "the connection broke (" + sizeText(written) + " of " + sizeText(length) + ")"; return false; }
            return written > 0;
        }
       #endif

        const juce::int64 expected;
        std::atomic<bool> cancelled{ false };
    };
}

class OrganCatalogContent final : public juce::Component, private juce::TableListBoxModel, private juce::Timer
{
public:
    explicit OrganCatalogContent(MainComponent& o) : owner(o)
    {
        cacheFolder = owner.settingsFile.getParentDirectory().getChildFile("Catalog");
        cacheFile = cacheFolder.getChildFile("catalog.json");

        intro.setText("The organs published for LibreOrgan Sampler. A click downloads the organ's configuration and loads it; the sample folders "
            "themselves must be on this computer (the last column says whether they are) - the README names their producer.",
            juce::dontSendNotification);
        intro.setFont(juce::FontOptions(15.0f));
        intro.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        intro.setJustificationType(juce::Justification::topLeft);
        intro.setMinimumHorizontalScale(1.0f);
        addAndMakeVisible(intro);

        filterLabel.setText("Filter", juce::dontSendNotification);
        filterLabel.setFont(juce::FontOptions(15.0f));
        addAndMakeVisible(filterLabel);
        filter.setTextToShowWhenEmpty("a part of the organ's name, a perspective ...", juce::Colours::grey);
        filter.setTooltip("Shows the organs whose name, perspectives or producer hold every word typed here.");
        filter.onTextChange = [this] { applyFilter(); };
        filter.getProperties().set("testName", "catalogFilter");
        addAndMakeVisible(filter);

        ramBox.addItem("RAM: any", 1);
        static const int limits[]{ 2, 4, 8, 16, 32, 64 };
        for (int i = 0; i < 6; ++i) ramBox.addItem("RAM up to " + juce::String(limits[i]) + " GB", 2 + i);
        ramBox.setSelectedId(1, juce::dontSendNotification);
        ramBox.setTooltip("Shows the organs that take no more RAM than this at the least - with every saving option of the load box "
            "(16-bit, 32 kHz on loading, releases at half the rate, packed samples). At the full quality count on roughly ten times as much.");
        ramBox.onChange = [this] { applyFilter(); };
        addAndMakeVisible(ramBox);

        hereOnly.setButtonText("Only organs whose samples are on this computer");
        hereOnly.setTooltip("Shows the organs whose sample folders the program finds on this computer - the same search a load does.");
        hereOnly.onClick = [this] { applyFilter(); };
        addAndMakeVisible(hereOnly);

        table.setModel(this);
        table.setRowHeight(28);
        table.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff2b2b2b));
        table.setOutlineThickness(1);
        table.getProperties().set("manualChapter", "The organ catalog");
        auto& header = table.getHeader();
        const int flags = juce::TableHeaderComponent::visible | juce::TableHeaderComponent::resizable | juce::TableHeaderComponent::sortable;
        header.addColumn("Organ", colOrgan, 380, 160, -1, flags);
        header.addColumn("Divisions", colDivisions, 90, 70, 140, flags);
        header.addColumn("Stops", colStops, 80, 60, 140, flags);
        header.addColumn("RAM", colRam, 90, 70, 140, flags);
        header.addColumn("Routes", colRoutes, 250, 100, -1, flags);
        header.addColumn("Samples on this computer", colSamples, 210, 150, -1, flags);
        header.setSortColumnId(colOrgan, true);
        header.setStretchToFitActive(true);
        addAndMakeVisible(table);

        status.setFont(juce::FontOptions(15.0f));
        status.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        status.setMinimumHorizontalScale(1.0f);
        addAndMakeVisible(status);

        loadButton.setButtonText("Download and load");
        loadButton.setTooltip("Downloads the chosen organ's configuration and loads it - the load box comes as with Load Configuration "
            "(a double-click on the row does the same). The sample folders must be on this computer.");
        loadButton.onClick = [this] { loadSelected(false); };
        addAndMakeVisible(loadButton);
        samplesButton.setButtonText("Download with its samples");
        samplesButton.setTooltip("For an organ whose samples are free: downloads the archive with the samples into Documents/LibreOrgan/LibreSamples "
            "and loads the organ from it. Nothing is downloaded twice.");
        samplesButton.onClick = [this] { loadSelected(true); };
        addAndMakeVisible(samplesButton);
        readmeButton.setButtonText("README");
        readmeButton.setTooltip("Opens the organ's page on GitHub in the browser: its samples (the producer and the link), the folder trees it expects, its Stop List.");
        readmeButton.onClick = [this] { if (const auto* e = selectedEntry()) if (e->readme.startsWithIgnoreCase("https://")) juce::URL(e->readme).launchInDefaultBrowser(); };
        addAndMakeVisible(readmeButton);
        refreshButton.setButtonText("Refresh");
        refreshButton.setTooltip("Reads the catalog from the internet again and looks for the samples on this computer again.");
        refreshButton.onClick = [this] { fetchCatalog(); };
        addAndMakeVisible(refreshButton);
        cancelButton.setButtonText("Cancel the download");
        cancelButton.setTooltip("Stops the download that runs; nothing of it is kept.");
        cancelButton.onClick = [this] { if (download != nullptr) download->cancel(); };
        addChildComponent(cancelButton);
        for (auto* button : { &loadButton, &samplesButton, &readmeButton, &refreshButton, &cancelButton })
            button->getProperties().set("manualChapter", "The organ catalog");

        // The catalog of the last time at once (also without the internet), the present one right after it.
        if (cacheFile.existsAsFile()) showCatalog(juce::JSON::parse(cacheFile.loadFileAsString()), true);
        fetchCatalog();
        startTimer(150);
    }

    ~OrganCatalogContent() override
    {
        stopTimer();
        lookCounter->store(++lookGeneration);   // a look still running stops and reports to nobody
        download.reset();
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(12);
        closeButton.place(*this, area);
        auto buttons = juce::Rectangle<int>(area.getX(), closeButton.getY() - 2, closeButton.getX() - area.getX() - 12, 36).reduced(0, 2);
        loadButton.setBounds(buttons.removeFromLeft(170));
        buttons.removeFromLeft(8);
        samplesButton.setBounds(buttons.removeFromLeft(300));
        buttons.removeFromLeft(8);
        readmeButton.setBounds(buttons.removeFromLeft(100));
        buttons.removeFromLeft(8);
        refreshButton.setBounds(buttons.removeFromLeft(100));
        buttons.removeFromLeft(8);
        cancelButton.setBounds(buttons.removeFromLeft(180));
        area.removeFromBottom(4);
        status.setBounds(area.removeFromBottom(24));
        area.removeFromBottom(4);
        intro.setBounds(area.removeFromTop(44));
        auto filters = area.removeFromTop(30);
        filterLabel.setBounds(filters.removeFromLeft(48));
        filter.setBounds(filters.removeFromLeft(juce::jmin(360, filters.getWidth() / 3)));
        filters.removeFromLeft(12);
        ramBox.setBounds(filters.removeFromLeft(170));
        filters.removeFromLeft(12);
        hereOnly.setBounds(filters);
        area.removeFromTop(8);
        table.setBounds(area);
        table.getHeader().resizeAllColumnsToFit(table.getVisibleContentWidth());   // (the columns share the width: never a bar across the bottom)
    }

    // The test hook's acts (ORGAN_TEST_CATALOG): what a click or a typed word does, and the state into the log.
    void testAct(const juce::StringArray& parts)
    {
        const auto& act = parts[1];
        if (act == "filter") { filter.setText(parts.size() > 2 ? parts[2] : juce::String(), false); applyFilter(); }
        else if (act == "ram") { ramBox.setSelectedId(juce::jlimit(1, 7, parts.size() > 2 ? parts[2].getIntValue() : 1), juce::sendNotificationSync); }
        else if (act == "here") { hereOnly.setToggleState(parts.size() > 2 && parts[2].getIntValue() != 0, juce::sendNotificationSync); }
        else if (act == "sort" && parts.size() > 2)
        {
            static const char* names[]{ "", "organ", "divisions", "stops", "ram", "routes", "samples" };
            for (int id = 1; id <= 6; ++id) if (parts[2] == names[id]) table.getHeader().setSortColumnId(id, !(parts.size() > 3 && parts[3] == "desc"));
        }
        else if (act == "refresh") fetchCatalog();
        else if ((act == "load" || act == "samples" || act == "select") && parts.size() > 2)
        {
            for (int row = 0; row < (int)shown.size(); ++row)
                if (all[(size_t)shown[(size_t)row]].folder.containsIgnoreCase(parts[2]))
                {
                    table.selectRow(row);
                    if (act != "select") loadSelected(act == "samples");
                    break;
                }
        }
        else if (act == "cancel") { if (download != nullptr) download->cancel(); }
        juce::StringArray lines;
        for (int row = 0; row < juce::jmin(6, (int)shown.size()); ++row)
        {
            const auto& e = all[(size_t)shown[(size_t)row]];
            lines.add(e.title + " [" + juce::String(e.divisions) + " div, " + juce::String(e.stops) + " stops, " + OrganCatalog::sizeText(e.ramBytes) + ", " + samplesText(e) + "]");
        }
        CrashHandler::note("ORGAN_TEST_CATALOG " + parts.joinIntoString("|") + ": " + juce::String((int)shown.size()) + " of " + juce::String((int)all.size())
            + " organ(s) shown; status \"" + status.getText() + "\"; first: " + lines.joinIntoString("; "));
    }

private:
    enum { colOrgan = 1, colDivisions, colStops, colRam, colRoutes, colSamples };
    enum class Job { none, catalog, configuration, samples };

    // ---- the table ----
    int getNumRows() override { return (int)shown.size(); }
    const OrganCatalog::Entry* entryOfRow(int row) const { return row >= 0 && row < (int)shown.size() ? &all[(size_t)shown[(size_t)row]] : nullptr; }
    const OrganCatalog::Entry* selectedEntry() const { return entryOfRow(table.getSelectedRow()); }

    void paintRowBackground(juce::Graphics& g, int row, int width, int height, bool selected) override
    {
        if (selected) g.fillAll(juce::Colour(0xff3d5a80));
        else if ((row & 1) != 0) g.fillAll(juce::Colours::white.withAlpha(0.03f));
        // (the line between the featured organs and all the others, as the website's two tables)
        const auto* e = entryOfRow(row);
        const auto* above = entryOfRow(row - 1);
        if (e != nullptr && above != nullptr && above->featured && !e->featured)
        {
            g.setColour(juce::Colours::grey.withAlpha(0.65f));
            g.fillRect(0, 0, width, 2);
        }
        juce::ignoreUnused(height);
    }

    juce::String samplesText(const OrganCatalog::Entry& e) const
    {
        if (e.samplesUrl.isNotEmpty() && samplesFileOf(e).existsAsFile() && e.treesHere <= 0) return "its archive is downloaded";
        if (e.trees.isEmpty()) return "-";
        if (e.treesHere < 0) return "looking ...";
        if (e.treesHere >= e.trees.size()) return "here";
        const juce::String free = e.samplesUrl.isNotEmpty() ? " - free: " + OrganCatalog::sizeText(e.samplesBytes) : juce::String();
        if (e.treesHere == 0) return "not here" + free;
        return juce::String(e.treesHere) + " of " + juce::String(e.trees.size()) + " folder trees" + free;
    }

    juce::String cellText(const OrganCatalog::Entry& e, int column) const
    {
        switch (column)
        {
            case colOrgan:     return e.title;
            case colDivisions: return juce::String(e.divisions);
            case colStops:     return juce::String(e.stops);
            case colRam:       return OrganCatalog::sizeText(e.ramBytes);
            case colRoutes:    return e.routes;
            case colSamples:   return samplesText(e);
            default:           return {};
        }
    }

    void paintCell(juce::Graphics& g, int row, int column, int width, int height, bool selected) override
    {
        const auto* e = entryOfRow(row);
        if (e == nullptr) return;
        juce::ignoreUnused(selected);
        auto colour = juce::Colours::white;
        if (column == colSamples)
            colour = e->treesHere >= e->trees.size() && !e->trees.isEmpty() ? juce::Colours::lightgreen
                   : e->treesHere > 0 || e->samplesUrl.isNotEmpty() ? juce::Colours::white : juce::Colours::grey;
        g.setColour(colour);
        g.setFont(juce::FontOptions(15.0f, column == colOrgan && e->featured ? juce::Font::bold : juce::Font::plain));
        const bool number = column == colDivisions || column == colStops || column == colRam;
        g.drawText(cellText(*e, column), juce::Rectangle<int>(8, 0, width - 16, height), number ? juce::Justification::centredRight : juce::Justification::centredLeft, true);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.fillRect(width - 1, 0, 1, height);
    }

    juce::String getCellTooltip(int row, int column) override
    {
        const auto* e = entryOfRow(row);
        if (e == nullptr) return {};
        if (column == colSamples)
        {
            juce::String text = e->treesHere < 0 ? juce::String("The program is looking for the organ's sample folders on this computer.")
                : juce::String(juce::jmax(0, e->treesHere)) + " of " + juce::String(e->trees.size()) + " folder tree(s) of the organ found on this computer.";
            text << "\nExpected under \"" << e->rootName << "\": " << e->trees.joinIntoString(", ");
            if (e->samplesUrl.isNotEmpty()) text << "\nIts samples are free: \"Download with its samples\" fetches them (" << OrganCatalog::sizeText(e->samplesBytes) << ").";
            return text;
        }
        if (column == colRam) return "The least the organ takes, with every saving option of the load box. At the full quality count on roughly ten times as much.";
        juce::String text = e->title + (e->featured ? "  (featured)" : "");
        if (e->producer.isNotEmpty()) text << "\nSamples: " << e->producer;
        text << "\nPerspectives: " << e->routes << "\nDouble-click: download and load.";
        return text;
    }

    void sortOrderChanged(int, bool) override { applyFilter(); }
    void selectedRowsChanged(int) override { updateButtons(); }
    void cellDoubleClicked(int row, int, const juce::MouseEvent&) override { table.selectRow(row); loadSelected(false); }
    void returnKeyPressed(int row) override { table.selectRow(row); loadSelected(false); }

    void applyFilter()
    {
        const auto chosen = selectedEntry() != nullptr ? selectedEntry()->folder : juce::String();
        const auto words = juce::StringArray::fromTokens(filter.getText().toLowerCase(), " ,;", "");
        static const int limits[]{ 0, 0, 2, 4, 8, 16, 32, 64 };
        const juce::int64 ramLimit = (juce::int64)limits[juce::jlimit(1, 7, ramBox.getSelectedId())] << 30;
        shown.clear();
        for (int i = 0; i < (int)all.size(); ++i)
        {
            const auto& e = all[(size_t)i];
            const auto text = (e.title + " " + e.routes + " " + e.producer + " " + e.folder).toLowerCase();
            bool fits = true;
            for (const auto& word : words) if (word.isNotEmpty() && !text.contains(word)) { fits = false; break; }
            if (ramLimit > 0 && (e.ramBytes <= 0 || e.ramBytes > ramLimit)) fits = false;   // (an unknown RAM passes no limit)
            if (hereOnly.getToggleState() && !(e.treesHere >= e.trees.size() && !e.trees.isEmpty())) fits = false;
            if (fits) shown.push_back(i);
        }
        const int column = table.getHeader().getSortColumnId();
        const bool forwards = table.getHeader().isSortedForwards();
        std::stable_sort(shown.begin(), shown.end(), [&](int a, int b)
        {
            const auto& x = all[(size_t)a];
            const auto& y = all[(size_t)b];
            if (x.featured != y.featured) return x.featured;   // the featured organs first, whatever the column
            int order = 0;
            switch (column)
            {
                case colDivisions: order = x.divisions - y.divisions; break;
                case colStops:     order = x.stops - y.stops; break;
                case colRam:       order = x.ramBytes < y.ramBytes ? -1 : x.ramBytes > y.ramBytes ? 1 : 0; break;
                case colRoutes:    order = x.routes.compareIgnoreCase(y.routes); break;
                case colSamples:
                {
                    auto share = [](const OrganCatalog::Entry& e) { return e.trees.isEmpty() ? -1.0 : (double)juce::jmax(0, e.treesHere) / (double)e.trees.size(); };
                    order = share(x) > share(y) ? -1 : share(x) < share(y) ? 1 : 0;   // (forwards: the organs that are here first)
                    break;
                }
                default: break;
            }
            if (order == 0) return x.title.compareIgnoreCase(y.title) < 0;
            return forwards ? order < 0 : order > 0;
        });
        if (column == colOrgan && !forwards)
            std::stable_sort(shown.begin(), shown.end(), [&](int a, int b)
            {
                const auto& x = all[(size_t)a];
                const auto& y = all[(size_t)b];
                if (x.featured != y.featured) return x.featured;
                return x.title.compareIgnoreCase(y.title) > 0;
            });
        table.updateContent();
        table.deselectAllRows();
        for (int row = 0; row < (int)shown.size(); ++row) if (all[(size_t)shown[(size_t)row]].folder == chosen) { table.selectRow(row); break; }
        table.repaint();
        updateButtons();
    }

    void updateButtons()
    {
        const auto* e = selectedEntry();
        const bool idle = job == Job::none;
        loadButton.setEnabled(idle && e != nullptr);
        const bool free = e != nullptr && e->samplesUrl.isNotEmpty();
        samplesButton.setEnabled(idle && free);
        samplesButton.setButtonText(free ? "Download with its samples (" + OrganCatalog::sizeText(e->samplesBytes) + ")" : juce::String("Download with its samples"));
        readmeButton.setEnabled(e != nullptr && e->readme.isNotEmpty());
        refreshButton.setEnabled(idle);
        cancelButton.setVisible(job == Job::configuration || job == Job::samples);
    }

    // ---- the catalog itself ----
    void showCatalog(const juce::var& parsed, bool fromCache)
    {
        auto entries = OrganCatalog::entriesOf(parsed);
        if (entries.empty()) return;
        // (what was found on this computer before stays shown until the new look ends)
        for (auto& e : entries)
            for (const auto& old : all)
                if (old.folder == e.folder && old.trees == e.trees) { e.treesHere = old.treesHere; break; }
        all = std::move(entries);
        catalogShown = true;
        if (fromCache) status.setText("The catalog of " + cacheFile.getLastModificationTime().toString(true, true, false) + " (kept on this computer) - asking for the present one ...", juce::dontSendNotification);
        applyFilter();
        lookForSamples();
    }

    void fetchCatalog()
    {
        if (job != Job::none) return;
        job = Job::catalog;
        download = std::make_unique<OrganCatalog::Download>(OrganCatalog::catalogLink(), cacheFolder.getChildFile("catalog-new.json"), 0);
        download->begin();
        if (!catalogShown) status.setText("Reading the catalog from the internet ...", juce::dontSendNotification);
        updateButtons();
    }

    void catalogArrived(bool ok, const juce::String& error, const juce::File& file)
    {
        juce::var parsed;
        if (ok) parsed = juce::JSON::parse(file.loadFileAsString());
        const bool good = ok && !OrganCatalog::entriesOf(parsed).empty();
        if (good)
        {
            cacheFile.deleteFile();
            file.moveFileTo(cacheFile);
            showCatalog(parsed, false);
            status.setText(juce::String((int)all.size()) + " organs in the catalog, read from the internet now.", juce::dontSendNotification);
            CrashHandler::note("Organ catalog: " + juce::String((int)all.size()) + " organ(s) read from " + OrganCatalog::catalogLink());
        }
        else
        {
            file.deleteFile();
            const auto why = ok ? juce::String("what came is no catalog") : error;
            status.setText(catalogShown ? "The catalog could not be read from the internet (" + why + ") - the one of "
                                            + cacheFile.getLastModificationTime().toString(true, true, false) + " is shown."
                                        : "The catalog could not be read from the internet: " + why + ".", juce::dontSendNotification);
            CrashHandler::note("Organ catalog: not read from " + OrganCatalog::catalogLink() + " - " + why);
        }
    }

    // ---- the samples on this computer: the search a load does, on a thread, quietly ----
    void lookForSamples()
    {
        if (owner.activeFolderLoader != nullptr || owner.searchingSampleFolders) { lookWanted = true; return; }   // (the search's caches are a load's just now)
        lookWanted = false;
        struct Ask { juce::String folder, rootName, rootRelative; juce::StringArray trees; juce::StringPairArray fingerprints; };
        std::vector<Ask> asks;
        for (const auto& e : all) asks.push_back({ e.folder, e.rootName, e.rootRelative, e.trees, e.fingerprints });
        const int generation = ++lookGeneration;
        lookCounter->store(generation);
        const auto beside = owner.configurationsDirectory().getChildFile("catalog.json");   // (where the configuration would stand)
        juce::Component::SafePointer<OrganCatalogContent> safe(this);
        auto counter = lookCounter;
        juce::Thread::launch([asks = std::move(asks), generation, beside, safe, counter]
        {
            const double began = juce::Time::getMillisecondCounterHiRes();
            for (const auto& ask : asks)
            {
                if (counter->load() != generation) return;   // the window went, or a newer look began
                const int here = MainComponent::catalogTreesHere(beside, ask.rootName, ask.rootRelative, ask.trees, ask.fingerprints);
                juce::MessageManager::callAsync([safe, generation, folder = ask.folder, here]
                {
                    if (safe == nullptr || safe->lookGeneration != generation) return;
                    for (auto& e : safe->all) if (e.folder == folder) e.treesHere = here;
                    if (safe->hereOnly.getToggleState() || safe->table.getHeader().getSortColumnId() == colSamples) safe->applyFilter(); else safe->table.repaint();
                });
            }
            const int ms = juce::roundToInt(juce::Time::getMillisecondCounterHiRes() - began);
            juce::MessageManager::callAsync([safe, generation, ms, count = (int)asks.size()]
            {
                if (safe == nullptr || safe->lookGeneration != generation) return;
                int here = 0;
                for (const auto& e : safe->all) if (e.treesHere >= e.trees.size() && !e.trees.isEmpty()) ++here;
                CrashHandler::note("Organ catalog: the samples of " + juce::String(here) + " of " + juce::String(count) + " organ(s) are on this computer (looked in " + juce::String(ms) + " ms)");
            });
        });
    }

    // ---- download and load ----
    juce::File samplesFileOf(const OrganCatalog::Entry& e) const
    {
        return owner.configurationsDirectory().getParentDirectory().getChildFile("LibreSamples").getChildFile(e.samplesName);
    }

    void loadSelected(bool withSamples)
    {
        const auto* e = selectedEntry();
        if (e == nullptr || job != Job::none) return;
        jobEntry = *e;
        if (withSamples && e->samplesUrl.isNotEmpty())
        {
            const auto file = samplesFileOf(*e);
            if (file.existsAsFile() && (e->samplesBytes <= 0 || file.getSize() == e->samplesBytes)) { samplesArrived(file); return; }
            job = Job::samples;
            download = std::make_unique<OrganCatalog::Download>(e->samplesUrl, file, e->samplesBytes);
        }
        else
        {
            job = Job::configuration;
            const auto folder = cacheFolder.getChildFile(juce::File::createLegalFileName(e->folder));
            download = std::make_unique<OrganCatalog::Download>(e->url, folder.getChildFile(juce::File::createLegalFileName(e->zip.isNotEmpty() ? e->zip : e->folder + ".zip")), 0);
        }
        download->begin();
        updateButtons();
    }

    void configurationArrived(const juce::File& zip)
    {
        // The zip holds the one configuration (the robot's check made sure of it when it was published; made sure again).
        juce::String failure;
        juce::File json;
        int pluginsOff = 0;
        {
            juce::ZipFile archive(zip);
            int index = -1;
            for (int i = 0; i < archive.getNumEntries(); ++i)
                if (const auto* entry = archive.getEntry(i); entry != nullptr && entry->filename.endsWithIgnoreCase(".json")) { index = i; break; }
            std::unique_ptr<juce::InputStream> in(index >= 0 ? archive.createStreamForEntry(index) : nullptr);
            juce::MemoryBlock bytes;
            if (in != nullptr) in->readIntoMemoryBlock(bytes);
            const auto text = bytes.toString();
            auto parsed = juce::JSON::parse(text);
            if (in == nullptr) failure = "the file is no zip with a configuration in it";
            else if (!parsed.isObject() || !parsed.hasProperty("manualKeyboards")) failure = "the zip holds no organ configuration";
            else
            {
                // Its plug-ins are another computer's choice: every one comes switched OFF (none is loaded until switched on).
                if (const auto* list = parsed.getProperty("perspectiveEffects", juce::var()).getArray())
                    for (const auto& item : *list)
                        if (const auto* plugins = item.getProperty("plugins", juce::var()).getArray())
                            for (const auto& plugin : *plugins)
                                if (auto* object = plugin.getDynamicObject()) { object->setProperty("on", false); ++pluginsOff; }
                json = zip.getSiblingFile(juce::File::createLegalFileName(archive.getEntry(index)->filename.fromLastOccurrenceOf("/", false, false).fromLastOccurrenceOf("\\", false, false)));
                const bool written = pluginsOff > 0 ? json.replaceWithText(juce::JSON::toString(parsed), false, false, "\n") : json.replaceWithData(bytes.getData(), bytes.getSize());
                if (!written) failure = "the configuration could not be written: " + json.getFullPathName();
            }
        }
        if (failure.isNotEmpty())
        {
            status.setText("\"" + jobEntry.title + "\" was not loaded: " + failure + ".", juce::dontSendNotification);
            CrashHandler::note("Organ catalog: \"" + jobEntry.title + "\" not loaded - " + failure + " (" + zip.getFullPathName() + ")");
            return;
        }
        status.setText("\"" + jobEntry.title + "\" downloaded - loading it.", juce::dontSendNotification);
        owner.loadFromCatalog(json, jobEntry.title, pluginsOff);
    }

    void samplesArrived(const juce::File& archive)
    {
        status.setText("The samples of \"" + jobEntry.title + "\" are on this computer now: " + archive.getFullPathName(), juce::dontSendNotification);
        owner.loadFromCatalogArchive(archive, jobEntry.title);
    }

    void timerCallback() override
    {
        if (lookWanted && owner.activeFolderLoader == nullptr && !owner.searchingSampleFolders) lookForSamples();
        if (download == nullptr) return;
        if (download->running())
        {
            if (job == Job::configuration || job == Job::samples)
            {
                const auto done = download->done.load();
                const auto total = download->total.load();
                juce::String text = "Downloading " + juce::String(job == Job::samples ? "the samples of \"" : "\"") + jobEntry.title + "\": " + OrganCatalog::sizeText(juce::jmax((juce::int64)1, done));
                if (total > 0) text << " of " << OrganCatalog::sizeText(total) << " (" << juce::roundToInt(100.0 * (double)done / (double)total) << " %)";
                status.setText(text, juce::dontSendNotification);
            }
            return;
        }
        // Ended: taken out of the member first - what follows may start the next download.
        const auto finished = std::move(download);
        const auto was = job;
        job = Job::none;
        updateButtons();
        if (was == Job::catalog) { catalogArrived(finished->ok, finished->error, finished->target); return; }
        if (finished->wasCancelled())
        {
            status.setText("The download was cancelled; nothing of it is kept.", juce::dontSendNotification);
            CrashHandler::note("Organ catalog: the download of \"" + jobEntry.title + "\" cancelled");
            return;
        }
        if (!finished->ok)
        {
            status.setText("\"" + jobEntry.title + "\" could not be downloaded: " + finished->error + ".", juce::dontSendNotification);
            CrashHandler::note("Organ catalog: \"" + jobEntry.title + "\" not downloaded from " + finished->link + " - " + finished->error);
            return;
        }
        CrashHandler::note("Organ catalog: " + finished->link + " downloaded to " + finished->target.getFullPathName() + " (" + OrganCatalog::sizeText(finished->target.getSize()) + ")");
        if (was == Job::samples) samplesArrived(finished->target); else configurationArrived(finished->target);
    }

    MainComponent& owner;
    juce::File cacheFolder, cacheFile;
    std::vector<OrganCatalog::Entry> all;
    std::vector<int> shown;
    bool catalogShown = false, lookWanted = false;
    int lookGeneration = 0;
    // (the look's thread reads this to learn that it is stale - the window may be gone by then)
    std::shared_ptr<std::atomic<int>> lookCounter = std::make_shared<std::atomic<int>>(0);
    Job job = Job::none;
    OrganCatalog::Entry jobEntry;
    std::unique_ptr<OrganCatalog::Download> download;

    juce::Label intro, filterLabel, status;
    juce::TextEditor filter;
    juce::ComboBox ramBox;
    juce::ToggleButton hereOnly;
    juce::TableListBox table;
    juce::TextButton loadButton, samplesButton, readmeButton, refreshButton, cancelButton;
    DialogCloseButton closeButton;
};
