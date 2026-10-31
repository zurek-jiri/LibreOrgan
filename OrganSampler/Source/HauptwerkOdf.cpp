#include "HauptwerkOdf.h"
#include "CrashHandler.h"
#include "SampleFiles.h"
#include "StopNames.h"

#include <algorithm>
#include <cstring>
#include <set>
#include <tuple>

namespace
{
    //==========================================================================
    // A row of an ODF table: the fields in file order. Looked up by the long
    // name or the compact letter (a compacted file writes only the letters).
    struct Row
    {
        std::vector<std::pair<juce::String, juce::String>> fields;
        const juce::String* find(const char* longName, const char* letter) const
        {
            for (const auto& f : fields) if (f.first == longName || (letter != nullptr && f.first == letter)) return &f.second;
            return nullptr;
        }
        juce::String text(const char* longName, const char* letter, const juce::String& fallback = {}) const
        {
            const auto* v = find(longName, letter);
            return v != nullptr ? *v : fallback;
        }
        int integer(const char* longName, const char* letter, int fallback = 0) const
        {
            const auto* v = find(longName, letter);
            if (v == nullptr || v->isEmpty()) return fallback;
            return (int)std::lround(v->getDoubleValue());   // compacted files write "1.27e+2"
        }
        float number(const char* longName, const char* letter, float fallback = 0.0f) const
        {
            const auto* v = find(longName, letter);
            return v == nullptr || v->isEmpty() ? fallback : (float)v->getDoubleValue();
        }
    };

    juce::String decodeEntities(const juce::String& text)
    {
        if (!text.containsChar('&')) return text;
        juce::String out;
        out.preallocateBytes((size_t)text.length());
        for (int i = 0; i < text.length(); ++i)
        {
            if (text[i] != '&') { out << text[i]; continue; }
            const int semicolon = text.indexOfChar(i, ';');
            if (semicolon < 0 || semicolon - i > 10) { out << text[i]; continue; }
            const auto entity = text.substring(i + 1, semicolon);
            if (entity == "amp") out << '&';
            else if (entity == "lt") out << '<';
            else if (entity == "gt") out << '>';
            else if (entity == "quot") out << '"';
            else if (entity == "apos") out << '\'';
            else if (entity.startsWithChar('#'))
            {
                const int code = entity[1] == 'x' || entity[1] == 'X' ? entity.substring(2).getHexValue32() : entity.substring(1).getIntValue();
                if (code > 0) out << juce::String::charToString((juce::juce_wchar)code);
            }
            else { out << text[i]; continue; }
            i = semicolon;
        }
        return out;
    }

    // The tables read, with the compact row tag "o" or the table's own name.
    // Every direct child element of an ObjectList is a row; every child of a
    // row is a field. The XML is scanned by hand: a 150 MB ODF is nothing to
    // put through a DOM.
    class Scanner
    {
    public:
        Scanner(const char* data, size_t size) : begin(data), end(data + size), p(data) {}

        // Advances to the next ObjectList and returns its ObjectType, or an empty string at the end.
        juce::String nextTable()
        {
            while (true)
            {
                const char* tag = findTag("<ObjectList");
                if (tag == nullptr) { p = end; return {}; }
                p = tag + 11;
                const char* close = (const char*)memchr(p, '>', (size_t)(end - p));
                if (close == nullptr) { p = end; return {}; }
                const juce::String head(p, (size_t)(close - p));
                p = close + 1;
                const int at = head.indexOf("ObjectType=\"");
                if (at < 0) continue;
                const auto rest = head.substring(at + 12);
                return rest.upToFirstOccurrenceOf("\"", false, false);
            }
        }
        void skipTable() { const char* e = findTag("</ObjectList>"); p = e != nullptr ? e + 13 : end; }
        double progress() const { return end > begin ? (double)(p - begin) / (double)(end - begin) : 1.0; }

        // The next row of the current table into 'row'; false at the table's end.
        bool nextRow(Row& row)
        {
            row.fields.clear();
            while (p < end)
            {
                const char* lt = (const char*)memchr(p, '<', (size_t)(end - p));
                if (lt == nullptr) { p = end; return false; }
                p = lt + 1;
                if (p >= end) return false;
                if (*p == '/')   // </ObjectList> or stray end tags
                {
                    if (startsWith(p + 1, "ObjectList")) { skipPastGreater(); return false; }
                    skipPastGreater();
                    continue;
                }
                if (*p == '!' || *p == '?') { skipPastGreater(); continue; }   // comments, declarations
                // A row element: read its name, then its fields up to the matching end tag.
                const char* nameStart = p;
                while (p < end && *p != '>' && *p != ' ' && *p != '/' && *p != '\t' && *p != '\r' && *p != '\n') ++p;
                const juce::String rowName(nameStart, (size_t)(p - nameStart));
                bool selfClosing = false;
                while (p < end && *p != '>') { if (*p == '/') selfClosing = true; ++p; }
                if (p < end) ++p;
                if (selfClosing) return true;   // an empty row
                readFields(row, rowName);
                return true;
            }
            return false;
        }

    private:
        const char* begin; const char* end; const char* p;

        bool startsWith(const char* at, const char* what) const
        {
            const size_t n = strlen(what);
            return (size_t)(end - at) >= n && memcmp(at, what, n) == 0;
        }
        void skipPastGreater()
        {
            const char* gt = (const char*)memchr(p, '>', (size_t)(end - p));
            p = gt != nullptr ? gt + 1 : end;
        }
        const char* findTag(const char* what) const
        {
            const size_t n = strlen(what);
            const char* s = p;
            while (s < end)
            {
                const char* lt = (const char*)memchr(s, '<', (size_t)(end - s));
                if (lt == nullptr) return nullptr;
                if ((size_t)(end - lt) >= n && memcmp(lt, what, n) == 0) return lt;
                s = lt + 1;
            }
            return nullptr;
        }
        void readFields(Row& row, const juce::String& rowName)
        {
            while (p < end)
            {
                const char* lt = (const char*)memchr(p, '<', (size_t)(end - p));
                if (lt == nullptr) { p = end; return; }
                p = lt + 1;
                if (p >= end) return;
                if (*p == '/')
                {
                    // The row's end tag (or any end tag at this level): the row is complete.
                    ++p;
                    const char* nameStart = p;
                    while (p < end && *p != '>') ++p;
                    const juce::String endName(nameStart, (size_t)(p - nameStart));
                    if (p < end) ++p;
                    if (endName.trim() == rowName || endName.startsWith("ObjectList")) return;
                    continue;
                }
                if (*p == '!' || *p == '?') { skipPastGreater(); continue; }
                const char* nameStart = p;
                while (p < end && *p != '>' && *p != ' ' && *p != '/' && *p != '\t') ++p;
                const juce::String fieldName(nameStart, (size_t)(p - nameStart));
                bool selfClosing = false;
                while (p < end && *p != '>') { if (*p == '/') selfClosing = true; ++p; }
                if (p < end) ++p;
                if (selfClosing) { row.fields.emplace_back(fieldName, juce::String()); continue; }
                // The text up to the field's end tag; CDATA taken as it is.
                const char* textStart = p;
                juce::String value;
                if (startsWith(p, "<![CDATA["))
                {
                    const char* cdataStart = p + 9;
                    const char* cdataEnd = cdataStart;
                    while (cdataEnd + 2 < end && !(cdataEnd[0] == ']' && cdataEnd[1] == ']' && cdataEnd[2] == '>')) ++cdataEnd;
                    value = juce::String::fromUTF8(cdataStart, (int)(cdataEnd - cdataStart));
                    p = juce::jmin(end, cdataEnd + 3);
                    const char* gt = (const char*)memchr(p, '>', (size_t)(end - p));
                    p = gt != nullptr ? gt + 1 : end;
                }
                else
                {
                    const char* textEnd = (const char*)memchr(p, '<', (size_t)(end - p));
                    if (textEnd == nullptr) { p = end; return; }
                    value = decodeEntities(juce::String::fromUTF8(textStart, (int)(textEnd - textStart)).trim());
                    p = textEnd;
                    skipPastGreater();   // the field's end tag
                }
                row.fields.emplace_back(fieldName, value);
            }
        }
    };

    bool containsFolderNamed(const juce::String& relativePath, const char* name)
    {
        for (const auto& part : juce::StringArray::fromTokens(relativePath.replaceCharacter('\\', '/'), "/", ""))
            if (part.equalsIgnoreCase(name)) return true;
        return false;
    }

    juce::File commonAncestor(juce::File a, const juce::File& b)
    {
        while (a != juce::File() && b != a && !b.isAChildOf(a))
        {
            const auto parent = a.getParentDirectory();
            if (parent == a) return {};
            a = parent;
        }
        return a;
    }

}

//==============================================================================
bool HauptwerkOdf::looksLikeOdf(const juce::File& file)
{
    if (!SampleFiles::existsAsFile(file)) return false;   // on the disk or in a RAR archive (0.8.157)
    if (file.getFileName().endsWithIgnoreCase(".Organ_Hauptwerk_xml")) return true;
    if (!file.hasFileExtension("xml")) return false;
    auto in = SampleFiles::openInput(file);
    if (in == nullptr) return false;
    char head[4096]{};
    const int n = in->read(head, (int)sizeof(head) - 1);
    return n > 0 && juce::String::fromUTF8(head, n).contains("<Hauptwerk");
}

juce::File HauptwerkOdf::hauptwerkRootFor(const juce::File& odf)
{
    // The folders may lie in RAR archives beside each other (0.8.157): SampleFiles sees them as folders.
    auto folder = odf.getParentDirectory();
    if (folder.getFileName().equalsIgnoreCase("OrganDefinitions") && SampleFiles::isDirectory(folder.getParentDirectory().getChildFile("OrganInstallationPackages")))
        return folder.getParentDirectory();
    for (int up = 0; up < 5 && folder != juce::File(); ++up)
    {
        if (SampleFiles::isDirectory(folder.getChildFile("OrganInstallationPackages"))) return folder;
        const auto parent = folder.getParentDirectory();
        if (parent == folder) break;
        folder = parent;
    }
    return {};
}

juce::File HauptwerkOdf::configurationFolderFor(const juce::File& hauptwerkRoot)
{
    // Beside the folder above HauptwerkSampleSetsAndComponents (the user's
    // "Hauptwerk" folder, whatever its name); a root without that level (a set
    // unpacked or archived on its own) puts it beside the root itself. A drive
    // root has no "beside": inside it then.
    const juce::File above = hauptwerkRoot.getFileName().equalsIgnoreCase("HauptwerkSampleSetsAndComponents") ? hauptwerkRoot.getParentDirectory() : hauptwerkRoot;
    juce::File beside = above.getParentDirectory();
    if (beside == juce::File() || beside == above) beside = above;
    return beside.getChildFile("OrganConfigurations");
}

juce::File HauptwerkOdf::packageFolder(const juce::File& hauptwerkRoot, int packageId)
{
    const auto packages = hauptwerkRoot.getChildFile("OrganInstallationPackages");
    const auto padded = packages.getChildFile(juce::String(packageId).paddedLeft('0', 6));
    if (SampleFiles::isDirectory(padded)) return padded;
    const auto bare = packages.getChildFile(juce::String(packageId));
    return SampleFiles::isDirectory(bare) ? bare : padded;
}

// A relative path with every part spelt as the folder on the disk is (a
// case-insensitive match per part where the exact name does not exist);
// empty when a part is not there at all.
juce::String HauptwerkOdf::spelledAsOnDisk(const juce::File& base, const juce::String& relative)
{
    juce::StringArray parts;
    parts.addTokens(relative.replaceCharacter('\\', '/'), "/", "");
    parts.removeEmptyStrings();
    juce::File dir = base;
    juce::StringArray spelt;
    for (const auto& part : parts)
    {
        juce::File next = dir.getChildFile(part);
        juce::String name = part;
        bool found = false;
        const auto children = SampleFiles::childDirectories(dir);   // disk or archive (0.8.157)
        for (const auto& child : children)
            if (child.getFileName() == part) { found = true; break; }
        if (!found)
            for (const auto& child : children)
                if (child.getFileName().equalsIgnoreCase(part)) { name = child.getFileName(); next = child; found = true; break; }
        if (!found) return {};
        spelt.add(name);
        dir = next;
    }
    return spelt.joinIntoString("/");
}

juce::String HauptwerkOdf::perspectiveOfRankName(const juce::String& name)
{
    // "Front A (direct): HW. Octave 2" - the part before the colon names the
    // perspective; its parenthesis is the most specific word, then the words.
    juce::String head = name.contains(":") ? name.upToFirstOccurrenceOf(":", false, false) : name;
    juce::String candidate;
    if (head.contains("(") && head.contains(")"))
        candidate = head.fromFirstOccurrenceOf("(", false, false).upToFirstOccurrenceOf(")", false, false);
    // The label is the perspective word ITSELF, capitalised - never a class of
    // synonyms: "Front C (distant)" and "Rear" are two perspectives of one set
    // (Ashton-under-Lyne: direct, diffuse, distant, rear), and a table that
    // read "distant" as Rear put two ranks into one perspective, which the
    // import then made a stop folder of (0.8.131).
    auto labelOf = [](const juce::String& text) -> juce::String
    {
        const auto lower = text.toLowerCase();
        static const char* const words[]{ "direct", "dry", "diffuse", "ambient", "ambience", "surround", "wet", "room", "hall", "rear", "back",
            "distant", "far", "hinten", "lointain", "front", "vorne", "close", "near", "nah", "proche", "middle", "mid", "center", "centre", "mitte",
            "gallery", "nave", "semi-dry", "semidry" };
        for (const auto* word : words)
        {
            const int at = lower.indexOf(word);
            if (at < 0) continue;
            // A whole word only ("mid" must not hit "middle"'s neighbours or "humid").
            const bool startOk = at == 0 || !juce::CharacterFunctions::isLetter(lower[at - 1]);
            const int after = at + (int)strlen(word);
            const bool endOk = after >= lower.length() || !juce::CharacterFunctions::isLetter(lower[after]);
            if (startOk && endOk) return juce::String(word).substring(0, 1).toUpperCase() + juce::String(word).substring(1);
        }
        return {};
    };
    auto label = labelOf(candidate);
    if (label.isEmpty() && candidate.trim().isNotEmpty() && candidate.trim().length() <= 16)
        label = candidate.trim().substring(0, 1).toUpperCase() + candidate.trim().substring(1);   // any short word in the parenthesis names the perspective
    if (label.isEmpty()) label = labelOf(head);
    // "Front A", "Front B" without a parenthesis: the letter tells them apart.
    if (label.isNotEmpty() && candidate.isEmpty())
    {
        const auto rest = head.trim().fromFirstOccurrenceOf(" ", false, false).trim();
        if (rest.length() == 1 && juce::CharacterFunctions::isLetterOrDigit(rest[0])) label << " " << rest.toUpperCase();
    }
    return label;
}

HauptwerkOdf HauptwerkOdf::parse(const juce::File& file, const std::function<bool(double)>& progress)
{
    HauptwerkOdf odf;
    juce::MemoryBlock data;
    // The definition from the disk, or out of a RAR archive (0.8.157).
    if (auto stream = SampleFiles::openInput(file)) stream->readIntoMemoryBlock(data);
    if (data.getSize() < 16) { odf.error = "The file could not be read."; return odf; }
    const char* text = (const char*)data.getData();
    if (juce::String::fromUTF8(text, (int)juce::jmin((size_t)4096, data.getSize())).indexOf("<Hauptwerk") < 0)
    {
        odf.error = "The file is not a Hauptwerk organ definition (no <Hauptwerk> element at its start).";
        return odf;
    }
    Scanner scanner(text, data.getSize());
    Row row;
    // Tables that refer to each other, in the order the file has them (any order).
    struct SampleRow { int packageId = 0; juce::String file; };
    std::map<int, SampleRow> samples;                 // SampleID -> file
    std::map<int, int> attackSampleOfLayer;           // LayerID -> SampleID (the first attack sample)
    std::map<int, int> releaseRowsOfLayer;            // LayerID -> release sample rows (none at all: a one-shot rank)
    struct LayerRow { int pipeId = 0; float levelDb = 0.0f; };
    std::map<int, LayerRow> layers;                   // LayerID -> pipe, level (file order: the first layer of a pipe comes first)
    std::vector<int> layerOrder;
    struct PipeRow { int rankId = 0, note = 60; };
    std::map<int, PipeRow> pipes;                     // PipeID
    std::vector<int> pipeOrder;
    struct StopRankRow { int stopId = 0, rankId = 0, alternateRankId = 0, firstNote = 0, count = 0, increment = 0; };
    std::vector<StopRankRow> stopRanks;
    std::map<int, std::vector<int>> keyboardNotes;    // KeyboardID -> notes of its KeyboardKey rows
    // The way from a key to a division: KeyboardKey.SwitchID -> SwitchLinkage
    // rows (source -> destination, possibly in several hops and under a
    // condition switch: a ventil, "Cp: Anches Pedal") -> DivisionInput.SwitchID.
    std::map<int, std::vector<int>> keyboardKeySwitches;             // KeyboardID -> the switches of its keys
    std::map<int, int> divisionOfInputSwitch;                        // DivisionInput switch -> DivisionID
    std::map<int, std::vector<std::pair<int, int>>> switchLinks;     // source switch -> (destination switch, condition switch or 0)
    // The tremulants: Tremulant (speed range) > TremulantWaveform (waveform ->
    // tremulant) > TremulantWaveformPipe (pipe -> waveform) - which pipes, so
    // which ranks, stops and divisions a tremulant works on.
    std::map<int, int> tremulantOfWaveform;                          // TremulantWaveformID -> TremulantID
    std::vector<std::pair<int, int>> tremulantPipes;                 // (PipeID, TremulantWaveformID)
    // The combinations: the crescendo is one per stage ("Cresc.Comb.12"); an
    // element = a switch the combination sets, on (Y) or off.
    std::map<int, juce::String> combinationNames;                    // CombinationID -> name
    std::map<int, juce::String> switchNames;                         // SwitchID -> name (the couplers' drawstops)
    std::map<int, juce::String> pageNames;                           // DisplayPageID -> name
    struct DrawknobRow { int page = 0, x = 0, y = 0; juce::String name; };
    std::vector<DrawknobRow> drawknobs;                              // ImageSetInstance rows (every image on every page; the stops are picked by name)
    struct ElementRow { int combinationId = 0, switchId = 0; bool on = false; };
    std::vector<ElementRow> combinationElements;
    int rows = 0;
    while (true)
    {
        const auto table = scanner.nextTable();
        if (table.isEmpty()) break;
        if (progress != nullptr && !progress(scanner.progress())) { odf.error = "Cancelled."; return odf; }
        const auto lower = table.toLowerCase();
        if (lower == "_general")
        {
            while (scanner.nextRow(row))
            {
                odf.organName = row.text("Identification_Name", nullptr);
                odf.builder = row.text("OrganInfo_Builder", nullptr);
                odf.location = row.text("OrganInfo_Location", nullptr);
                odf.supplier = row.text("Control_OrganDefinitionSupplierName", nullptr);
                odf.mainPackageId = row.integer("OrganInfo_InstallationPackageID", nullptr, 0);
                odf.compacted = row.text("Control_FileIsCompacted_AlwaysSetThisToNIfEditingManually", nullptr).equalsIgnoreCase("Y");
            }
        }
        else if (lower == "requiredinstallationpackage")
        {
            while (scanner.nextRow(row))
            {
                const int id = row.integer("InstallationPackageID", "a");
                if (id > 1 && std::find(odf.packageIds.begin(), odf.packageIds.end(), id) == odf.packageIds.end()) odf.packageIds.push_back(id);
            }
        }
        else if (lower == "division")
        {
            while (scanner.nextRow(row))
            {
                Division d; d.id = row.integer("DivisionID", "a"); d.name = row.text("Name", "b");
                if (d.id > 0) odf.divisions[d.id] = d;
            }
        }
        else if (lower == "keyboard")
        {
            while (scanner.nextRow(row))
            {
                Keyboard k;
                k.id = row.integer("KeyboardID", "a");
                k.name = row.text("Name", "b");
                k.divisionId = row.integer("Hint_PrimaryAssociatedDivisionID", "x");
                const int keys = row.integer("KeyGen_NumberOfKeys", "h", 0), first = row.integer("KeyGen_MIDINoteNumberOfFirstKey", "i", 0);
                if (keys > 0 && first > 0) { k.lowNote = first; k.highNote = first + keys - 1; }
                if (k.id > 0) odf.keyboards.push_back(k);
            }
        }
        else if (lower == "keyboardkey")
        {
            while (scanner.nextRow(row))
            {
                const int keyboard = row.integer("KeyboardID", "a"), note = row.integer("NormalMIDINoteNumber", "c", -1);
                if (keyboard > 0 && note >= 0) keyboardNotes[keyboard].push_back(note);
                const int keySwitch = row.integer("SwitchID", "b");
                if (keyboard > 0 && keySwitch > 0) keyboardKeySwitches[keyboard].push_back(keySwitch);
            }
        }
        else if (lower == "divisioninput")
        {
            while (scanner.nextRow(row))
            {
                const int division = row.integer("DivisionID", "a"), inputSwitch = row.integer("SwitchID", "b");
                if (division > 0 && inputSwitch > 0) divisionOfInputSwitch[inputSwitch] = division;
            }
        }
        else if (lower == "displaypage")
        {
            while (scanner.nextRow(row))
            {
                const int id = row.integer("PageID", "a");
                if (id > 0) pageNames[id] = row.text("Name", "b");
            }
        }
        else if (lower == "imagesetinstance")
        {
            // Every image on every page: b = name ("2. clairon 4 (StopPage)"), e = the
            // page, g/h = its position in pixels. The stops are picked by name below.
            while (scanner.nextRow(row))
            {
                DrawknobRow d;
                d.name = row.text("Name", "b");
                d.page = row.integer("DisplayPageID", "e");
                d.x = row.integer("LeftXPosPixels", "g");
                d.y = row.integer("TopYPosPixels", "h");
                if (d.page > 0 && d.name.isNotEmpty()) drawknobs.push_back(d);
            }
        }
        else if (lower == "switch")
        {
            while (scanner.nextRow(row))
            {
                const int id = row.integer("SwitchID", "a");
                if (id > 0) switchNames[id] = row.text("Name", "b");
            }
        }
        else if (lower == "keyaction")
        {
            // A coupler: a = the keyboard played, c = the keyboard it plays as well (or
            // d = the division), f = the condition switch (the drawstop), n = the
            // semitones (12 = a 4' coupler, -12 = 16'), h = 3 on a melody coupler
            // (Bellevue, Billerbeck, Caen: compacted letters; the long names are a guess
            // for an uncompacted file and fall back to the letters).
            while (scanner.nextRow(row))
            {
                Coupler c;
                c.sourceKeyboardId = row.integer("SourceKeyboardID", "a");
                c.destKeyboardId = row.integer("DestKeyboardID", "c");
                c.destDivisionId = row.integer("DestDivisionID", "d");
                c.switchId = row.integer("ConditionSwitchID", "f");
                c.shift = row.integer("MIDINoteNumIncrementToDestination", "n", 0);
                const int actionType = row.integer("ActionTypeCode", "h", 0);   // 2 = the lowest key only (a bass coupler), 3 = the highest (melody)
                c.melody = actionType == 3;
                c.bass = actionType == 2;
                c.name = row.text("Name", "e");
                if (c.sourceKeyboardId > 0 && c.switchId > 0 && (c.destKeyboardId > 0 || c.destDivisionId > 0)) odf.couplers.push_back(c);
            }
        }
        else if (lower == "switchlinkage")
        {
            while (scanner.nextRow(row))
            {
                const int source = row.integer("SourceSwitchID", "a"), dest = row.integer("DestSwitchID", "b"), condition = row.integer("ConditionSwitchID", "c");
                if (source > 0 && dest > 0) switchLinks[source].push_back({ dest, condition });
            }
        }
        else if (lower == "tremulant")
        {
            while (scanner.nextRow(row))
            {
                Tremulant t;
                t.id = row.integer("TremulantID", "a");
                t.name = row.text("Name", "b");
                // The speed wanders between a minimum and a maximum (Hz): the middle is the rate.
                const float lowHz = row.number("ModRateHz_Min", "d", 0.0f), highHz = row.number("ModRateHz_Max", "e", 0.0f);
                t.rateHz = lowHz > 0.0f && highHz > 0.0f ? 0.5f * (lowHz + highHz) : juce::jmax(lowHz, highHz);
                if (t.id > 0) odf.tremulants.push_back(t);
            }
        }
        else if (lower == "tremulantwaveform")
        {
            while (scanner.nextRow(row))
            {
                const int waveform = row.integer("TremulantWaveformID", "a"), tremulant = row.integer("TremulantID", "c");
                if (waveform > 0 && tremulant > 0) tremulantOfWaveform[waveform] = tremulant;
            }
        }
        else if (lower == "tremulantwaveformpipe")
        {
            while (scanner.nextRow(row))
            {
                const int pipe = row.integer("PipeID", "a"), waveform = row.integer("TremulantWaveformID", "b");
                if (pipe > 0 && waveform > 0) tremulantPipes.push_back({ pipe, waveform });
            }
        }
        else if (lower == "combination")
        {
            while (scanner.nextRow(row))
            {
                const int id = row.integer("CombinationID", "a");
                if (id > 0) combinationNames[id] = row.text("Name", "b");
            }
        }
        else if (lower == "combinationelement")
        {
            while (scanner.nextRow(row))
            {
                ElementRow e;
                e.combinationId = row.integer("CombinationID", "b");
                e.switchId = row.integer("ControlledSwitchID", "c");
                e.on = row.text("InitialStoredStateIsOn", "e").equalsIgnoreCase("Y");
                if (e.combinationId > 0 && e.switchId > 0) combinationElements.push_back(e);
            }
        }
        else if (lower == "stop")
        {
            while (scanner.nextRow(row))
            {
                Stop s; s.id = row.integer("StopID", "a"); s.name = row.text("Name", "b"); s.divisionId = row.integer("DivisionID", "c");
                s.switchId = row.integer("ControllingSwitchID", "d");
                if (s.id > 0) odf.stops[s.id] = s;
            }
        }
        else if (lower == "stoprank")
        {
            while (scanner.nextRow(row))
            {
                StopRankRow r; r.stopId = row.integer("StopID", "a"); r.rankId = row.integer("RankID", "d"); r.alternateRankId = row.integer("AlternateRankID", "p");
                // The key mapping of the row: the first key, how many keys, the
                // semitones from the key to the pipe (an extension borrows another
                // stop's rank with 12 or -12 here).
                r.firstNote = row.integer("MIDINoteNumOfFirstMappedDivisionInputNode", "h");
                r.count = row.integer("NumberOfMappedDivisionInputNodes", "i");
                r.increment = row.integer("MIDINoteNumIncrementFromDivisionToRank", "j");
                if (r.stopId > 0 && r.rankId > 0) stopRanks.push_back(r);
            }
        }
        else if (lower == "rank")
        {
            while (scanner.nextRow(row))
            {
                Rank r; r.id = row.integer("RankID", "a"); r.name = row.text("Name", "b");
                if (r.id > 0) odf.ranks[r.id] = r;
            }
        }
        else if (lower == "pipe_soundengine01")
        {
            while (scanner.nextRow(row))
            {
                const int id = row.integer("PipeID", "a");
                PipeRow pr; pr.rankId = row.integer("RankID", "b"); pr.note = row.integer("NormalMIDINoteNumber", "d", 60);
                if (id > 0 && pipes.find(id) == pipes.end()) { pipes[id] = pr; pipeOrder.push_back(id); }
            }
        }
        else if (lower == "pipe_soundengine01_layer")
        {
            while (scanner.nextRow(row))
            {
                const int id = row.integer("LayerID", "a");
                LayerRow lr; lr.pipeId = row.integer("PipeID", "b"); lr.levelDb = row.number("AmpLvl_LevelAdjustDecibels", "h", 0.0f);
                if (id > 0 && layers.find(id) == layers.end()) { layers[id] = lr; layerOrder.push_back(id); }
            }
        }
        else if (lower == "pipe_soundengine01_releasesample")
        {
            while (scanner.nextRow(row))
            {
                const int layer = row.integer("LayerID", "b");
                if (layer > 0) ++releaseRowsOfLayer[layer];
            }
        }
        else if (lower == "pipe_soundengine01_attacksample")
        {
            while (scanner.nextRow(row))
            {
                const int layer = row.integer("LayerID", "b"), sample = row.integer("SampleID", "c");
                if (layer > 0 && sample > 0 && attackSampleOfLayer.find(layer) == attackSampleOfLayer.end()) attackSampleOfLayer[layer] = sample;
            }
        }
        else if (lower == "sample")
        {
            while (scanner.nextRow(row))
            {
                const int id = row.integer("SampleID", "a");
                SampleRow sr; sr.packageId = row.integer("InstallationPackageID", "b"); sr.file = row.text("SampleFilename", "c").replaceCharacter('\\', '/');
                if (id > 0) samples[id] = sr;
                if ((++rows & 4095) == 0 && progress != nullptr && !progress(scanner.progress())) { odf.error = "Cancelled."; return odf; }
            }
        }
        else scanner.skipTable();
    }
    if (odf.mainPackageId > 1 && std::find(odf.packageIds.begin(), odf.packageIds.end(), odf.mainPackageId) == odf.packageIds.end())
        odf.packageIds.insert(odf.packageIds.begin(), odf.mainPackageId);
    if (odf.stops.empty() || pipes.empty() || samples.empty())
    {
        odf.error = "The organ definition holds no stops with sample pipes.";
        return odf;
    }

    // The keyboards' compasses from their keys (the compact form leaves the
    // KeyGen fields out); the pedal 36-67, a manual 36-96 as the last resort.
    for (auto& k : odf.keyboards)
    {
        const auto notes = keyboardNotes.find(k.id);
        if (notes != keyboardNotes.end() && !notes->second.empty())
        {
            k.lowNote = *std::min_element(notes->second.begin(), notes->second.end());
            k.highNote = *std::max_element(notes->second.begin(), notes->second.end());
        }
        if (k.lowNote <= 0 || k.highNote < k.lowNote) { k.lowNote = 36; k.highNote = k.name.containsIgnoreCase("ped") ? 67 : 96; }
    }

    // The divisions every keyboard plays: from each key switch along the switch
    // linkages (any number of hops; a condition switch - a ventil, "Cp: Anches
    // Recit" - makes the link optional but it is the keyboard's division all the
    // same) to the division inputs. Caen: four keyboards, eight divisions
    // ("Pedal" + "PedalAnches" and so on): a reed division reached only with its
    // ventil drawn is still that keyboard's. A division reached from several
    // keyboards (a coupler built as a linkage) goes to the keyboard reaching it
    // with the most keys without a condition, then with the most keys at all;
    // the ODF's own hint stays the primary one when it is among them. A keyboard
    // without any linkage (an old or hand-written definition) keeps the hint.
    {
        struct Reach { int unconditional = 0, total = 0; };
        std::map<int, std::map<int, Reach>> reachOfKeyboard;   // keyboard -> division -> keys reaching it
        std::map<int, std::map<int, int>> conditionOfKeyboardDivision;   // keyboard -> division -> the condition switch of the conditional links (0.8.144)
        for (const auto& k : odf.keyboards)
        {
            const auto keys = keyboardKeySwitches.find(k.id);
            if (keys == keyboardKeySwitches.end()) continue;
            for (const int keySwitch : keys->second)
            {
                // Depth first from the key: every division input reached, and whether
                // some link on the way was conditional.
                std::set<int> seen;
                std::map<int, bool> conditionalArrival;   // division -> arrived only through a condition
                std::vector<std::tuple<int, bool, int>> stack{ { keySwitch, false, 0 } };   // switch, conditional so far, the condition switch
                while (!stack.empty())
                {
                    const auto [sw, conditional, conditionId] = stack.back(); stack.pop_back();
                    if (!seen.insert(sw).second) continue;
                    if (sw != keySwitch)
                    {
                        const auto input = divisionOfInputSwitch.find(sw);
                        if (input != divisionOfInputSwitch.end())
                        {
                            auto arrived = conditionalArrival.find(input->second);
                            if (arrived == conditionalArrival.end()) conditionalArrival[input->second] = conditional;
                            else arrived->second = arrived->second && conditional;
                            // The condition the keys reach the division under (a unison off: the
                            // keyboard's own division plays only while that switch is off).
                            if (conditional && conditionId > 0 && conditionOfKeyboardDivision[k.id].count(input->second) == 0) conditionOfKeyboardDivision[k.id][input->second] = conditionId;
                        }
                    }
                    const auto links = switchLinks.find(sw);
                    if (links == switchLinks.end() || seen.size() > 4096) continue;
                    for (const auto& [dest, condition] : links->second) stack.push_back({ dest, conditional || condition != 0, conditionId != 0 ? conditionId : condition });
                }
                for (const auto& [division, conditional] : conditionalArrival)
                {
                    auto& reach = reachOfKeyboard[k.id][division];
                    ++reach.total;
                    if (!conditional) ++reach.unconditional;
                }
            }
        }
        std::map<int, int> ownerOfDivision;   // division -> keyboard
        for (const auto& [division, d] : odf.divisions)
        {
            int best = 0; Reach bestReach;
            for (const auto& k : odf.keyboards)
            {
                const auto reach = reachOfKeyboard.count(k.id) > 0 && reachOfKeyboard.at(k.id).count(division) > 0 ? reachOfKeyboard.at(k.id).at(division) : Reach();
                if (reach.total == 0) continue;
                const bool better = best == 0 || reach.unconditional > bestReach.unconditional
                    || (reach.unconditional == bestReach.unconditional && (reach.total > bestReach.total || (reach.total == bestReach.total && k.divisionId == division)));
                if (better) { best = k.id; bestReach = reach; }
            }
            if (best > 0) ownerOfDivision[division] = best;
        }
        for (auto& k : odf.keyboards)
        {
            std::vector<std::pair<Reach, int>> mine;   // (reach, division) of the divisions this keyboard owns
            for (const auto& [division, owner] : ownerOfDivision)
                if (owner == k.id) mine.push_back({ reachOfKeyboard.at(k.id).at(division), division });
            if (mine.empty()) { if (k.divisionId > 0) k.divisionIds = { k.divisionId }; continue; }
            // The primary: the hint when it is among them, else the division most
            // keys play without a condition; the rest by division id.
            std::stable_sort(mine.begin(), mine.end(), [](const auto& a, const auto& b)
            {
                return a.first.unconditional != b.first.unconditional ? a.first.unconditional > b.first.unconditional
                     : a.first.total != b.first.total ? a.first.total > b.first.total : a.second < b.second;
            });
            int primary = mine.front().second;
            for (const auto& m : mine) if (m.second == k.divisionId) primary = k.divisionId;
            // The keyboard's own division reached only under a condition switch: that
            // switch is its unison off (Ashton: "1. unison off" conditions the link from
            // the keys of 1. manual to the Choir), a coupler of ours (0.8.144).
            for (const auto& m : mine)
                if (m.second == primary && m.first.unconditional == 0)
                    if (const auto conds = conditionOfKeyboardDivision.find(k.id); conds != conditionOfKeyboardDivision.end())
                        if (const auto c = conds->second.find(primary); c != conds->second.end())
                        {
                            Coupler off;
                            off.sourceKeyboardId = k.id; off.destKeyboardId = k.id; off.switchId = c->second; off.unisonOff = true;
                            off.name = k.name + " unison off";
                            odf.couplers.push_back(off);
                        }
            k.divisionIds.clear();
            k.unconditionalDivisionIds.clear();
            for (const auto& m : mine) if (m.first.unconditional > 0) k.unconditionalDivisionIds.insert(m.second);
            k.divisionIds.push_back(primary);
            std::vector<int> rest;
            for (const auto& m : mine) if (m.second != primary) rest.push_back(m.second);
            std::sort(rest.begin(), rest.end());
            for (const int division : rest) k.divisionIds.push_back(division);
            k.divisionId = primary;
        }
    }

    // The stops' ranks (alternate = tremmed ranks marked, never taken).
    std::set<int> alternateRanks;
    for (const auto& sr : stopRanks) if (sr.alternateRankId > 0) alternateRanks.insert(sr.alternateRankId);
    for (const auto& sr : stopRanks)
    {
        auto stop = odf.stops.find(sr.stopId);
        if (stop == odf.stops.end()) continue;
        if (std::find(stop->second.rankIds.begin(), stop->second.rankIds.end(), sr.rankId) == stop->second.rankIds.end())
            stop->second.rankIds.push_back(sr.rankId);
        stop->second.rankUses.push_back({ sr.rankId, sr.firstNote, sr.count, sr.increment });
    }
    for (auto& [id, rank] : odf.ranks)
    {
        const auto lower = rank.name.toLowerCase();
        rank.tremmed = alternateRanks.count(id) > 0 || lower.contains("tremmed") || lower.contains("trem.") || lower.endsWith(" trem") || lower.contains("(trem");
        // Noises by whole words of the name ("Key action noise", "Blower",
        // "Stop action", "Tracker"): a substring test took "I. Zimbell" for a bell.
        juce::StringArray words;
        {
            juce::String word;
            for (int i = 0; i < lower.length(); ++i)
            {
                const auto c = lower[i];
                if (juce::CharacterFunctions::isLetter(c)) word << juce::String::charToString(c);
                else if (word.isNotEmpty()) { words.add(word); word.clear(); }
            }
            if (word.isNotEmpty()) words.add(word);
        }
        static const char* const noiseWords[] = { "noise", "noises", "calcant", "blower", "action", "tracker", "trackers",
            "zimbelstern", "cymbelstern", "cimbelstern", "nightingale", "cuckoo", "ruch" };
        rank.noise = false;
        for (const auto* noiseWord : noiseWords) if (words.contains(noiseWord)) rank.noise = true;
        if (lower.contains("noise")) rank.noise = true;   // "keynoise", "stopnoise"
        rank.perspective = perspectiveOfRankName(rank.name);
    }

    // Every pipe's first layer and its first attack sample: the file, the level.
    const auto root = hauptwerkRootFor(file);
    std::set<int> pipesDone;
    std::map<int, int> rankReleaseRows;   // RankID -> release sample rows of its first layers
    for (const int layerId : layerOrder)
    {
        const auto& layer = layers[layerId];
        if (layer.pipeId <= 0 || pipesDone.count(layer.pipeId) > 0) continue;   // the pipe's further layers (tremmed and the like) are not taken
        const auto pipeIt = pipes.find(layer.pipeId);
        if (pipeIt == pipes.end()) continue;
        pipesDone.insert(layer.pipeId);
        const auto attack = attackSampleOfLayer.find(layerId);
        if (attack == attackSampleOfLayer.end()) continue;
        const auto sample = samples.find(attack->second);
        if (sample == samples.end() || sample->second.file.isEmpty()) continue;
        auto rankIt = odf.ranks.find(pipeIt->second.rankId);
        if (rankIt == odf.ranks.end()) continue;
        ++odf.sampleCount;
        const auto& path = sample->second.file;
        if (path.endsWithIgnoreCase(".hbw") || path.endsWithIgnoreCase(".hbx")) ++odf.encryptedSamples;
        if (containsFolderNamed(path, "trem")) continue;   // a tremmed sample: left out
        Pipe pipe;
        pipe.id = layer.pipeId; pipe.rankId = pipeIt->second.rankId; pipe.note = pipeIt->second.note;
        pipe.levelDb = layer.levelDb; pipe.packageId = sample->second.packageId; pipe.file = path;
        rankIt->second.pipes.push_back(pipe);
        if (releaseRowsOfLayer.count(layerId) > 0) rankReleaseRows[pipe.rankId] += releaseRowsOfLayer[layerId];
    }
    // A rank whose pipes have no release sample at all is a one-shot (chimes,
    // harp, carillon): the sample plays out whatever the key does.
    for (auto& [id, rank] : odf.ranks) rank.oneShot = !rank.pipes.empty() && rankReleaseRows[id] == 0;
    // The files' numbers against the pipes' notes (0.8.155): Doesburg names the
    // file of the pipe on key 36 "024" - the most common difference is the rank's
    // offset; the Stop List row carries it and the loader puts the files on their keys.
    for (auto& [id, rank] : odf.ranks)
    {
        std::map<int, int> counts;
        for (const auto& pipe : rank.pipes)
        {
            const auto name = pipe.file.fromLastOccurrenceOf("/", false, false);
            int number = 0, digits = 0;
            for (const auto c : name) { if (juce::CharacterFunctions::isDigit(c) && digits < 3) { number = number * 10 + (int)(c - '0'); ++digits; } else break; }
            if (digits >= 2) ++counts[number - pipe.note];
        }
        int best = 0, bestCount = -1;
        for (const auto& [offset, count] : counts) if (count > bestCount) { best = offset; bestCount = count; }
        rank.sampleOffset = bestCount > 0 ? juce::jlimit(-48, 48, best) : 0;
    }
    // Every rank's folder: the parent folder of most of its pipes, made absolute
    // through its installation package.
    for (auto& [id, rank] : odf.ranks)
    {
        std::map<juce::String, int> count;
        for (const auto& pipe : rank.pipes)
        {
            const auto dir = pipe.file.upToLastOccurrenceOf("/", false, false);
            ++count[juce::String(pipe.packageId) + "|" + dir];
        }
        juce::String best; int bestCount = 0;
        for (const auto& [key, n] : count) if (n > bestCount) { best = key; bestCount = n; }
        if (best.isEmpty() || root == juce::File()) continue;
        const int packageId = best.upToFirstOccurrenceOf("|", false, false).getIntValue();
        const auto dir = best.fromFirstOccurrenceOf("|", false, false);
        rank.packageId = packageId;
        rank.folderRelative = dir;
        rank.folder = packageFolder(root, packageId).getChildFile(dir.isEmpty() ? juce::String(".") : dir);
        // The folder as it is spelt ON THE DISK (0.8.149, #57): the definition says
        // "002293/ped", the disk has "Ped" - Windows does not care, Linux does, and
        // the configuration must carry the disk's spelling so that it loads on both.
        if (const auto onDisk = spelledAsOnDisk(packageFolder(root, packageId), dir); onDisk.isNotEmpty() && onDisk != dir)
        {
            rank.folderRelative = onDisk;
            rank.folder = packageFolder(root, packageId).getChildFile(onDisk);
        }
    }
    // Ranks of one stop that landed in ONE perspective label although their
    // folders carry different prefixes ("02-dir22-", "04-dis2h-", "05-reaSen-"
    // beside "03-dif22-"): the prefixes tell them apart, so they are different
    // perspectives whatever the names said - never one perspective, never a
    // stop folder of sub-stops (the user's rule: four prefixes, four
    // perspectives). The label becomes the old one plus the prefix's core.
    {
        auto prefixCore = [](juce::String prefix)
        {
            while (prefix.isNotEmpty() && !juce::CharacterFunctions::isLetter(prefix[0])) prefix = prefix.substring(1);
            while (prefix.isNotEmpty() && !juce::CharacterFunctions::isLetterOrDigit(prefix.getLastCharacter())) prefix = prefix.dropLastCharacters(1);
            return prefix;
        };
        for (const auto& [stopId, stop] : odf.stops)
        {
            std::map<juce::String, std::vector<int>> byLabel;
            for (const auto& use : stop.rankUses)
            {
                if (use.increment != 0) continue;
                const auto rankIt = odf.ranks.find(use.rankId);
                if (rankIt == odf.ranks.end() || rankIt->second.folder == juce::File() || rankIt->second.tremmed || rankIt->second.noise) continue;
                auto& list = byLabel[rankIt->second.perspective];
                if (std::find(list.begin(), list.end(), use.rankId) == list.end()) list.push_back(use.rankId);
            }
            for (auto& [label, rankIds] : byLabel)
            {
                if (rankIds.size() < 2) continue;
                std::vector<juce::String> names;
                for (const int id : rankIds) names.push_back(odf.ranks.at(id).folder.getFileName());
                // The longest common ending of the folder names is the stop's part;
                // what stands before it in each name is that rank's prefix.
                int common = 0;
                while (true)
                {
                    bool same = common < names.front().length();
                    const auto c = same ? names.front()[names.front().length() - 1 - common] : 0;
                    for (const auto& n : names) if (common >= n.length() || n[n.length() - 1 - common] != c) same = false;
                    if (!same) break;
                    ++common;
                }
                // A prefix stands BEFORE a common ending of some length; names that
                // differ at their end ("quintaton16" / "quintaton16l", Caen's L and R
                // components) are no perspectives but one stop's pieces: a stop folder.
                if (common < 3) continue;
                std::set<juce::String> prefixes;
                std::vector<juce::String> cores;
                for (const auto& n : names) { const auto core = prefixCore(n.dropLastCharacters(common)); cores.push_back(core); prefixes.insert(core.toLowerCase()); }
                if (prefixes.size() != rankIds.size()) continue;   // the same prefix twice (an L/R split): a stop folder, as before
                for (size_t i = 0; i < rankIds.size(); ++i)
                {
                    if (cores[i].isEmpty()) continue;
                    auto& rank = odf.ranks.at(rankIds[i]);
                    rank.perspective = (label.isEmpty() ? juce::String() : label + " ") + cores[i];
                }
            }
        }
    }
    for (auto& c : odf.couplers)
        if (const auto name = switchNames.find(c.switchId); name != switchNames.end()) c.switchName = name->second;
    // The jamb page: the images named like a stop (the name without its trailing
    // parentheses - "(StopPage)", "(NonPhoto)"), on the page holding the most of
    // them (Ashton: "Simple Jamb" with all 47; the Left/Right Jamb pages hold a
    // part each).
    {
        std::map<juce::String, int> stopByName;
        for (const auto& [id, stop] : odf.stops) stopByName[stop.name.trim().toLowerCase()] = id;
        std::map<int, std::map<int, StopPosition>> byPage;
        for (const auto& d : drawknobs)
        {
            juce::String base = d.name.trim();
            while (base.endsWithChar(')') && base.lastIndexOfChar('(') > 0) base = base.upToLastOccurrenceOf("(", false, false).trim();
            // The definition cuts an image name at 32 characters ("GO. Flute a cheminee 8 (NonPhoto",
            // 0.8.160, #80): an unclosed parenthesis goes too - 14 Bellevue stops had no cell and
            // took the first free cells instead of their places on the jamb.
            if (base.lastIndexOfChar('(') > 0 && base.indexOfChar(base.lastIndexOfChar('('), ')') < 0) base = base.upToLastOccurrenceOf("(", false, false).trim();
            const auto stop = stopByName.find(base.toLowerCase());
            if (stop == stopByName.end()) continue;
            auto& page = byPage[d.page];
            if (page.count(stop->second) == 0) page[stop->second] = { d.x, d.y };
        }
        // The SIMPLE jamb page whenever there is one (the definitions carry sister
        // pages, photorealistic and simple, with the same drawknobs - the simple one
        // is the plain layout); among several the one with the most stops; a page
        // of any name only when no "Simple" page exists.
        int bestPage = 0; size_t bestCount = 0; bool bestSimple = false;
        for (const auto& [page, stops] : byPage)
        {
            const bool simple = pageNames.count(page) > 0 && pageNames[page].containsIgnoreCase("simple");
            if ((simple && !bestSimple) || (simple == bestSimple && stops.size() > bestCount)) { bestCount = stops.size(); bestPage = page; bestSimple = simple; }
        }
        if (bestPage > 0)
        {
            odf.stopPositions = byPage[bestPage];
            odf.jambPageName = pageNames.count(bestPage) > 0 ? pageNames[bestPage] : juce::String("page " + juce::String(bestPage));
        }
    }
    // The crescendo: every combination whose name says "cresc" is one stage
    // (the number at the end of the name: "Cresc.Comb.12"); its elements set
    // switches, which reach the stops' own switches through the switch
    // linkages (Doesburg, Schwerin: "CrescendoCombSwitch ..." switches linked
    // to the stops), either way, within a few hops. An element reaching no stop
    // (a coupler, a tremulant) is left out.
    {
        std::map<int, int> stopOfSwitch;
        for (const auto& [id, stop] : odf.stops) if (stop.switchId > 0) stopOfSwitch[stop.switchId] = id;
        std::map<int, std::vector<int>> neighbours;   // both directions of the linkages
        for (const auto& [source, links] : switchLinks)
            for (const auto& [dest, condition] : links) { neighbours[source].push_back(dest); neighbours[dest].push_back(source); }
        auto stopOf = [&](int switchId) -> int
        {
            std::set<int> seen{ switchId };
            std::vector<int> frontier{ switchId };
            for (int hop = 0; hop < 8 && !frontier.empty(); ++hop)   // Schwerin: 4 hops (CrescendoCombSwitch > CrescendoIfOnSwitch > ... > C.<stop> > the stop)
            {
                for (const int s : frontier) { const auto it = stopOfSwitch.find(s); if (it != stopOfSwitch.end()) return it->second; }
                std::vector<int> next;
                for (const int s : frontier)
                    for (const int t : neighbours[s]) if (seen.insert(t).second) next.push_back(t);
                frontier.swap(next);
            }
            return 0;
        };
        // The couplers a stage draws: the element's switch reaching a coupler's
        // drawstop the same way (0.8.138).
        std::set<int> couplerSwitches;
        for (const auto& c : odf.couplers) couplerSwitches.insert(c.switchId);
        auto couplerSwitchOf = [&](int switchId) -> int
        {
            std::set<int> seen{ switchId };
            std::vector<int> frontier{ switchId };
            for (int hop = 0; hop < 8 && !frontier.empty(); ++hop)
            {
                for (const int s : frontier) if (couplerSwitches.count(s) > 0) return s;
                std::vector<int> next;
                for (const int s : frontier)
                    for (const int t : neighbours[s]) if (seen.insert(t).second) next.push_back(t);
                frontier.swap(next);
            }
            return 0;
        };
        std::map<int, std::set<int>> couplersOnByStage;
        std::map<int, std::set<int>> stopsOnByStage;
        std::map<int, int> stageOfCombination;
        for (const auto& [id, name] : combinationNames)
        {
            if (!name.containsIgnoreCase("cresc")) continue;
            const auto digits = name.trim().getTrailingIntValue();
            if (digits <= 0) continue;
            stageOfCombination[id] = digits;
            stopsOnByStage[digits];   // a stage with nothing drawn is a stage all the same
        }
        if (!stageOfCombination.empty())
        {
            std::map<int, int> stopCache;
            for (const auto& e : combinationElements)
            {
                const auto stage = stageOfCombination.find(e.combinationId);
                if (stage == stageOfCombination.end() || !e.on) continue;
                auto cached = stopCache.find(e.switchId);
                const int stopId = cached != stopCache.end() ? cached->second : (stopCache[e.switchId] = stopOf(e.switchId));
                if (stopId > 0) { stopsOnByStage[stage->second].insert(stopId); continue; }
                if (const int couplerSwitch = couplerSwitchOf(e.switchId); couplerSwitch > 0) couplersOnByStage[stage->second].insert(couplerSwitch);
            }
            for (const auto& [stage, ids] : stopsOnByStage)
            {
                CrescendoStep step{ stage, std::vector<int>(ids.begin(), ids.end()), {} };
                if (const auto c = couplersOnByStage.find(stage); c != couplersOnByStage.end()) step.couplerSwitchIds.assign(c->second.begin(), c->second.end());
                odf.crescendo.push_back(step);
            }
        }
    }
    // Which divisions every tremulant works on: its waveforms' pipes -> their
    // ranks -> the stop playing the rank straight -> that stop's division.
    {
        std::map<int, int> ownerOfRank;
        for (const auto& sr : stopRanks) if (sr.increment == 0 && ownerOfRank.count(sr.rankId) == 0) ownerOfRank[sr.rankId] = sr.stopId;
        for (const auto& sr : stopRanks) if (ownerOfRank.count(sr.rankId) == 0) ownerOfRank[sr.rankId] = sr.stopId;
        for (const auto& [pipeId, waveformId] : tremulantPipes)
        {
            const auto tremulant = tremulantOfWaveform.find(waveformId);
            const auto pipe = pipes.find(pipeId);
            if (tremulant == tremulantOfWaveform.end() || pipe == pipes.end()) continue;
            const auto owner = ownerOfRank.find(pipe->second.rankId);
            if (owner == ownerOfRank.end()) continue;
            const auto stop = odf.stops.find(owner->second);
            if (stop == odf.stops.end()) continue;
            for (auto& t : odf.tremulants) if (t.id == tremulant->second) t.divisionIds.insert(stop->second.divisionId);
        }
    }
    if (progress != nullptr)
        CrashHandler::note("Hauptwerk ODF parsed: divisions " + juce::String((int)odf.divisions.size()) + ", keyboards " + juce::String((int)odf.keyboards.size())
            + ", keyboard keys " + juce::String((int)keyboardNotes.size()) + ", stops " + juce::String((int)odf.stops.size()) + ", stop ranks " + juce::String((int)stopRanks.size())
            + ", ranks " + juce::String((int)odf.ranks.size()) + ", pipes " + juce::String((int)pipes.size()) + ", layers " + juce::String((int)layers.size())
            + ", attack samples " + juce::String((int)attackSampleOfLayer.size()) + ", samples " + juce::String((int)samples.size())
            + ", packages " + juce::String((int)odf.packageIds.size()));
    return odf;
}

//==============================================================================
HauptwerkOdf::Plan HauptwerkOdf::plan(const juce::File& hauptwerkRoot) const
{
    Plan result;
    result.organName = organName.isNotEmpty() ? organName : juce::String("Hauptwerk organ");
    if (error.isNotEmpty()) { result.error = error; return result; }
    if (hauptwerkRoot == juce::File()) { result.error = "The Hauptwerk folders were not found: the organ definition must lie in OrganDefinitions beside OrganInstallationPackages."; return result; }
    result.libraryRoot = hauptwerkRoot.getChildFile("OrganInstallationPackages");
    result.crescendo = crescendo;

    // The divisions in keyboard order, the pedal first: a keyboard whose name
    // (or division's name) says "ped", else the first keyboard with at most 32
    // keys; the rest keep the file's order.
    std::vector<Keyboard> ordered;
    std::set<int> divisionsUsed;
    auto isPedal = [this](const Keyboard& k)
    {
        const auto divisionName = divisions.count(k.divisionId) > 0 ? divisions.at(k.divisionId).name : juce::String();
        // A keyboard of more than 40 keys is a manual whatever its division is called
        // (0.8.159: Adlington's "2. manual" plays the division named "Pedal").
        if (k.highNote - k.lowNote + 1 > 40) return false;
        return k.name.containsIgnoreCase("ped") || divisionName.containsIgnoreCase("ped");
    };
    // Every division a keyboard plays (the ODF links them through switches,
    // Keyboard::divisionIds) is part of the keyboard's division of ours.
    auto divisionsOf = [](const Keyboard& k)
    {
        std::vector<int> ids = k.divisionIds;
        if (ids.empty() && k.divisionId > 0) ids.push_back(k.divisionId);
        return ids;
    };
    // A division reached by several keyboards belongs to the one with the most
    // keys (0.8.159: Adlington's "Pedal" keyboard of 25 keys and its "2. manual"
    // both play the division "Pedal" - the manual owns it, the pedal keyboard is
    // a pull-down and stays an empty P.; before, the file's first keyboard took
    // it and the organ came out as P. + I. instead of I. + II.).
    std::map<int, int> ownerOfDivision;   // ODF division id -> keyboard id
    for (const auto& k : keyboards)
        for (const int id : divisionsOf(k))
        {
            const auto found = ownerOfDivision.find(id);
            auto keysOf = [this](int keyboardId)
            {
                for (const auto& other : keyboards) if (other.id == keyboardId) return other.highNote - other.lowNote + 1;
                return 0;
            };
            if (found == ownerOfDivision.end() || keysOf(k.id) > keysOf(found->second)) ownerOfDivision[id] = k.id;
        }
    Keyboard pedalPullDown;   // a pedal keyboard left without a division of its own
    bool pedalPullDownFound = false;
    for (const auto& k : keyboards)
    {
        std::vector<int> ids;
        for (const int id : divisionsOf(k)) if (divisionsUsed.count(id) == 0 && ownerOfDivision[id] == k.id) ids.push_back(id);
        if (ids.empty())
        {
            if (isPedal(k) && !pedalPullDownFound && !divisionsOf(k).empty())
            {
                pedalPullDown = k; pedalPullDownFound = true;
                const int id = divisionsOf(k).front();
                juce::String ownerName;
                for (const auto& other : keyboards) if (other.id == ownerOfDivision[id]) ownerName = other.name;
                result.notes.add("keyboard \"" + k.name + "\" plays the division \"" + (divisions.count(id) > 0 ? divisions.at(id).name : juce::String(id))
                    + "\" of the keyboard \"" + ownerName + "\" (more keys): division P. stays empty, couple it from that manual");
            }
            continue;   // a second keyboard of a division (a floating one, a pull-down) adds nothing
        }
        const auto divisionName = divisions.count(k.divisionId) > 0 ? divisions.at(k.divisionId).name : juce::String();
        bool hasStops = false;
        for (const auto& [id, stop] : stops) if (std::find(ids.begin(), ids.end(), stop.divisionId) != ids.end()) { hasStops = true; break; }
        // A pedal without stops (a demo) stays - one of them: division P. is the
        // pedal or empty, never a manual; a second pedal keyboard without stops
        // (a split pedal) is left out.
        bool pedalKept = false;
        for (const auto& kept : ordered) if (isPedal(kept)) pedalKept = true;
        if (!hasStops && (!isPedal(k) || pedalKept)) { result.notes.add("division \"" + divisionName + "\" (keyboard \"" + k.name + "\") has no stops: left out"); continue; }
        for (const int id : ids) divisionsUsed.insert(id);
        Keyboard kept = k;
        kept.divisionIds = ids;
        if (std::find(ids.begin(), ids.end(), kept.divisionId) == ids.end()) kept.divisionId = ids.front();
        ordered.push_back(kept);
    }
    // The manuals from the lowest up: a number at the start of the keyboard's
    // name ("1. manual [lowest]", "2. manual") wins, else the keyboard ID (the
    // file lists the keyboards in any order).
    std::stable_sort(ordered.begin(), ordered.end(), [](const Keyboard& a, const Keyboard& b)
    {
        auto rankOf = [](const Keyboard& k)
        {
            const auto digits = k.name.trim().initialSectionContainingOnly("0123456789");
            return digits.isNotEmpty() ? digits.getIntValue() : 1000 + k.id;
        };
        return rankOf(a) < rankOf(b);
    });
    int pedalIndex = -1;
    for (size_t i = 0; i < ordered.size() && pedalIndex < 0; ++i) if (isPedal(ordered[i])) pedalIndex = (int)i;
    for (size_t i = 0; i < ordered.size() && pedalIndex < 0; ++i)
        if (ordered[i].highNote - ordered[i].lowNote + 1 <= 32) pedalIndex = (int)i;
    if (pedalIndex > 0) std::rotate(ordered.begin(), ordered.begin() + pedalIndex, ordered.begin() + pedalIndex + 1);
    if (pedalIndex < 0)
    {
        // No pedal (a harpsichord, a demo): division P. stays empty, the manuals begin at I.
        Keyboard empty; empty.name = "Pedal"; empty.lowNote = 36; empty.highNote = 67;
        if (pedalPullDownFound) { empty.name = pedalPullDown.name; empty.lowNote = pedalPullDown.lowNote; empty.highNote = pedalPullDown.highNote; }
        empty.divisionId = 0; empty.divisionIds.clear();
        ordered.insert(ordered.begin(), empty);
        if (!pedalPullDownFound) result.notes.add("no pedal in the organ definition: division P. stays empty");
    }
    if (ordered.size() > 8) { result.notes.add(juce::String((int)ordered.size() - 8) + " division(s) beyond eight left out"); ordered.resize(8); }
    bool anyStops = false;
    for (const auto& k : ordered)
        for (const int divisionId : divisionsOf(k))
            for (const auto& [id, stop] : stops) if (divisionId > 0 && stop.divisionId == divisionId) anyStops = true;
    if (!anyStops) { result.error = "No keyboard with stops was found in the organ definition."; return result; }
    // The rows that carry folders: the stops without children, and the children
    // of the stops that have them (a stop folder's own row has no folders).
    auto leavesOf = [](PlanDivision& division)
    {
        std::vector<PlanStop*> leaves;
        for (auto& stop : division.stops)
        {
            if (stop.children.empty()) { leaves.push_back(&stop); continue; }
            for (auto& child : stop.children) leaves.push_back(&child);
        }
        return leaves;
    };

    // Who owns a rank - among the stops that play it without a transposition:
    // (a) the stop whose name is the rank's name without its perspective head
    //     ("GO. Bombarde 16" owns "Rear: GO. Bombarde 16", whatever the file
    //     order; the pedal's "Ped. Bombarde 16" borrows it - not the other way
    //     round, as the first-in-file rule had it in Bellevue, 0.8.135);
    // (b) else the stop playing it over its whole division (no key count),
    //     then the one with the most keys, then the first in the file.
    // A rank nobody plays straight belongs to its first user.
    std::map<int, int> ownerOfRank;
    {
        struct Claim { int stopId; bool nameMatch; int count; int order; };
        std::map<int, std::vector<Claim>> claims;
        int order = 0;
        for (const auto& [id, stop] : stops)
        {
            const auto stopKey = stop.name.trim().toLowerCase();
            for (const auto& use : stop.rankUses)
            {
                if (use.increment != 0) continue;
                const auto rankIt = ranks.find(use.rankId);
                juce::String tail = rankIt != ranks.end() ? rankIt->second.name : juce::String();
                if (tail.contains(":")) tail = tail.fromFirstOccurrenceOf(":", false, false);
                claims[use.rankId].push_back({ id, tail.trim().toLowerCase() == stopKey, use.count, order++ });
            }
        }
        for (auto& [rankId, list] : claims)
        {
            std::stable_sort(list.begin(), list.end(), [](const Claim& a, const Claim& b)
            {
                if (a.nameMatch != b.nameMatch) return a.nameMatch;
                if ((a.count == 0) != (b.count == 0)) return a.count == 0;
                if (a.count != b.count) return a.count > b.count;
                return a.order < b.order;
            });
            ownerOfRank[rankId] = list.front().stopId;
        }
        for (const auto& [id, stop] : stops)
            for (const auto& use : stop.rankUses)
                if (ownerOfRank.count(use.rankId) == 0) ownerOfRank[use.rankId] = id;
    }
    auto usableRank = [this](int rankId) -> const Rank*
    {
        const auto rankIt = ranks.find(rankId);
        if (rankIt == ranks.end()) return nullptr;
        const auto& rank = rankIt->second;
        if (rank.tremmed || rank.noise) return nullptr;   // counted below, over the whole definition
        if (rank.pipes.empty() || rank.folder == juce::File()) return nullptr;
        return &rank;
    };
    // The stop folder(s) of a set of ranks: the perspectives of a stop lie side
    // by side under it, possibly spread over several installation packages (the
    // rear perspective in its own package) with the same paths below each - so
    // the paths relative to the packages are compared: their common part is the
    // stop folder in every package concerned. A single rank is its own stop
    // folder unless its name ends with its parent's ("03-rearMk4-PEd_Princip16"
    // under "PEd_Princip16": the parent is the stop). A common part shorter than
    // two folders means the perspectives lie ABOVE the stops: every rank folder
    // is then a stop folder of its own.
    auto foldersFor = [&](const std::vector<int>& rankIdsIn) -> std::vector<juce::File>
    {
        std::vector<juce::String> relatives;
        std::set<int> packages;
        std::vector<juce::File> rankFolders;
        for (const int rankId : rankIdsIn)
        {
            const auto& rank = ranks.at(rankId);
            relatives.push_back(rank.folderRelative); packages.insert(rank.packageId); rankFolders.push_back(rank.folder);
        }
        if (relatives.empty()) return {};
        auto components = [](const juce::String& path) { return juce::StringArray::fromTokens(path, "/", ""); };
        juce::StringArray common = components(relatives.front());
        for (const auto& rel : relatives)
        {
            const auto parts = components(rel);
            int n = 0;
            while (n < common.size() && n < parts.size() && common[n].equalsIgnoreCase(parts[n])) ++n;
            common.removeRange(n, common.size() - n);
        }
        if (relatives.size() == 1)
        {
            const auto parts = components(relatives.front());
            if (parts.size() >= 2 && parts[parts.size() - 1].endsWithIgnoreCase(parts[parts.size() - 2]) && parts[parts.size() - 1].length() > parts[parts.size() - 2].length())
                common.removeRange(common.size() - 1, 1);
        }
        std::vector<juce::File> folders;
        if (common.size() >= 2)
        {
            const auto stopRelative = common.joinIntoString("/");
            for (const int packageId : packages)
            {
                const auto folder = packageFolder(hauptwerkRoot, packageId).getChildFile(stopRelative);
                if (SampleFiles::isDirectory(folder)) folders.push_back(folder);
            }
        }
        if (folders.empty()) folders = rankFolders;   // the perspectives above the stops, or nothing in common
        return folders;
    };
    std::set<int> plannedStopIds;
    // One division of ours per keyboard, empty for now; the stops come next.
    for (const auto& k : ordered)
    {
        PlanDivision division;
        division.divisionId = k.divisionId;
        division.divisionIds = divisionsOf(k);
        division.name = divisions.count(k.divisionId) > 0 ? divisions.at(k.divisionId).name : k.name;
        // The definition's tremulant on this division (one modulating any of its
        // ODF divisions): its speed becomes the division's tremulant rate.
        for (const auto& t : tremulants)
            for (const int divisionId : division.divisionIds)
                if (division.tremulantRateHz <= 0.0f && t.rateHz > 0.0f && t.divisionIds.count(divisionId) > 0) division.tremulantRateHz = t.rateHz;
        division.lowNote = k.lowNote; division.highNote = k.highNote;
        result.divisions.push_back(division);
    }
    // Which division of ours a stop goes to: the keyboard playing its ODF
    // division - unless that division is a FLOATING one (no keyboard reaches it
    // without a condition: Bellevue's "Chamade", played from every keyboard
    // through a coupler) and the stop's name says whose it is ("GO. Trompette
    // en chamade 8", "Ped. Clairon en chamade", "5. Tuba 16"): then the
    // division that name points to, where the definition's console has it.
    std::set<int> floatingDivisions;
    for (const auto& [divisionId, d] : divisions)
    {
        bool unconditional = false;
        for (const auto& k : keyboards) if (k.unconditionalDivisionIds.count(divisionId) > 0) unconditional = true;
        if (!unconditional) floatingDivisions.insert(divisionId);
    }
    auto romanValue = [](const juce::String& text)
    {
        static const struct { const char* roman; int value; } table[]{ { "viii", 8 }, { "vii", 7 }, { "vi", 6 }, { "iv", 4 }, { "v", 5 }, { "iii", 3 }, { "ii", 2 }, { "i", 1 } };
        for (const auto& t : table) if (text == t.roman) return t.value;
        return 0;
    };
    auto divisionIndexByPrefix = [&](const juce::String& stopName) -> int
    {
        // The token before the first space, up to its dot or colon: "GO." "Ped." "1." "III." "Rec:".
        const auto head = stopName.trim();
        int cut = 0;
        while (cut < head.length() && head[cut] != ' ' && head[cut] != ':') ++cut;
        juce::String token = head.substring(0, cut).trimCharactersAtEnd(".:").toLowerCase();
        if (token.isEmpty() || token.length() > 8) return -1;
        auto pedalIndex = [&]() -> int { for (size_t i = 0; i < ordered.size(); ++i) if (isPedal(ordered[i]) || (i == 0 && ordered[i].divisionId <= 0)) return (int)i; return -1; };
        if (token == "p" || token == "ped" || token == "pd" || token == "pedal" || token == "pedale" || token == "pédale") return pedalIndex();
        int number = token.containsOnly("0123456789") ? token.getIntValue() : romanValue(token);
        if (number > 0)
        {
            // Manual N is division N of ours when the pedal is division 0 (it always is: an empty one stands in).
            return number < (int)ordered.size() ? number : -1;
        }
        struct Synonym { const char* token; const char* words; };
        static const Synonym synonyms[]{
            { "go", "great grand haupt" }, { "gt", "great grand haupt" }, { "g.o", "great grand haupt" }, { "hw", "haupt great grand" }, { "gr", "great grand" },
            { "sw", "swell schwell recit récit" }, { "rec", "recit récit swell schwell" }, { "réc", "récit recit swell" }, { "rez", "recit récit" }, { "schw", "schwell swell" },
            { "pos", "positif positiv" }, { "rp", "rück ruck positiv" }, { "bw", "brust" }, { "ow", "ober" }, { "ch", "choir" }, { "cho", "choir" },
            { "so", "solo" }, { "solo", "solo" }, { "ech", "echo fern" }, { "echo", "echo fern" }, { "cham", "chamade" }, { "bom", "bombarde" } };
        juce::StringArray words;
        for (const auto& syn : synonyms) if (token == syn.token) words.addTokens(syn.words, " ", "");
        for (size_t i = 0; i < ordered.size(); ++i)
        {
            juce::StringArray names{ ordered[i].name.toLowerCase() };
            for (const int divisionId : divisionsOf(ordered[i])) if (divisions.count(divisionId) > 0) names.add(divisions.at(divisionId).name.toLowerCase());
            for (const auto& name : names)
            {
                if (name.isEmpty()) continue;
                if (name.startsWith(token) && token.length() >= 2) return (int)i;
                juce::String initials;
                for (const auto& word : juce::StringArray::fromTokens(name, " -_", "")) if (word.isNotEmpty()) initials << word[0];
                if (initials == token && token.length() >= 2) return (int)i;
                for (const auto& word : words) if (name.contains(word)) return (int)i;
            }
        }
        return -1;
    };
    auto divisionIndexOf = [&](const Stop& stop) -> int
    {
        int index = -1;
        for (size_t i = 0; i < ordered.size() && index < 0; ++i)
        {
            const auto ids = divisionsOf(ordered[i]);
            if (std::find(ids.begin(), ids.end(), stop.divisionId) != ids.end()) index = (int)i;
        }
        if (floatingDivisions.count(stop.divisionId) > 0)
        {
            const int named = divisionIndexByPrefix(stop.name);
            if (named >= 0 && named != index)
            {
                result.notes.add("\"" + stop.name + "\": its division \"" + (divisions.count(stop.divisionId) > 0 ? divisions.at(stop.divisionId).name : juce::String())
                    + "\" is a floating one - placed on " + (named == 0 ? juce::String("P.") : juce::String(named) + ".") + " as its name says");
                index = named;
            }
        }
        return index;
    };
    // The stops in the order of the divisions of ours, each primary ODF division's first.
    int offsetStops = 0;   // stops whose files are numbered off their keys (0.8.155)
    std::vector<std::pair<int, int>> stopOrder;   // (division of ours, ODF stop id)
    for (size_t i = 0; i < ordered.size(); ++i)
        for (const int divisionId : divisionsOf(ordered[i]))
            for (const auto& [id, stop] : stops)
                if (stop.divisionId == divisionId)
                {
                    const int target = divisionIndexOf(stop);
                    if (target >= 0 && target < (int)result.divisions.size()) stopOrder.push_back({ target, id });
                }
    std::stable_sort(stopOrder.begin(), stopOrder.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    // A rank folder is loaded ONCE (0.8.160): a later stop whose ranks all lie in
    // folders an earlier stop already plays (Adlington's "Bl. Flute Bass" and "Bl.
    // Flute Treble": two ODF ranks, one folder) becomes a transmission of that
    // stop with the keys of its own rows.
    std::map<juce::String, int> ownerOfFolder;   // lower-case folder path -> ODF stop id
    for (const auto& [targetIndex, id] : stopOrder)
    {
        auto& division = result.divisions[(size_t)targetIndex];
        const auto& k = ordered[(size_t)targetIndex];
        const auto& stop = stops.at(id);
        const int divisionKeys = juce::jmax(1, k.highNote - k.lowNote + 1);
        // The stop's own ranks (the ones it owns) and what it borrows: owner
        // stop -> transposition -> keys. A stop with ranks of its own is a stop
        // of its own, whatever it borrows besides (the Tuba 16 of Billerbeck
        // plays its own rank and takes its top three keys from the Tuba 4: the
        // filled pipes cover those); only a stop with NO rank of its own is an
        // extension/transmission - of the owner it borrows the most keys from.
        std::vector<juce::File> rankFolders;
        std::vector<int> rankIds;
        std::map<int, std::map<int, int>> borrowed;
        for (const auto& use : stop.rankUses)
        {
            const auto* rank = usableRank(use.rankId);
            if (rank == nullptr) continue;
            const int owner = ownerOfRank.count(use.rankId) > 0 ? ownerOfRank.at(use.rankId) : id;
            if (owner != id)
            {
                // The shift of the borrowed piece RELATIVE TO THE OWNER'S own mapping of
                // that rank (0.8.143): the owner plays it at increment 0 as a rule, but a
                // rank nobody plays straight belongs to its first user, who may map it
                // shifted - the extension then differs by the two increments, as the
                // definition's key-to-pipe rows say, not by its own increment alone.
                int ownerIncrement = 0;
                if (const auto ownerStop = stops.find(owner); ownerStop != stops.end())
                {
                    bool found = false;
                    for (const auto& ownerUse : ownerStop->second.rankUses)
                        if (ownerUse.rankId == use.rankId && (!found || std::abs(ownerUse.increment) < std::abs(ownerIncrement))) { ownerIncrement = ownerUse.increment; found = true; }
                }
                borrowed[owner][use.increment - ownerIncrement] += use.count > 0 ? use.count : divisionKeys;
                continue;
            }
            if (std::find(rankIds.begin(), rankIds.end(), use.rankId) != rankIds.end()) continue;
            rankFolders.push_back(rank->folder);
            rankIds.push_back(use.rankId);
        }
        if (!rankFolders.empty())
        {
            // Every folder of its own already played by an earlier stop: a second
            // ODF rank on the same samples - the stop plays that stop's pipes.
            int folderOwner = 0;
            bool allOwned = true;
            for (const auto& folder : rankFolders)
            {
                const auto found = ownerOfFolder.find(folder.getFullPathName().toLowerCase());
                if (found == ownerOfFolder.end() || (folderOwner != 0 && found->second != folderOwner)) { allOwned = false; break; }
                folderOwner = found->second;
            }
            if (allOwned && folderOwner > 0 && folderOwner != id)
            {
                int shift = 0;
                for (const auto& use : stop.rankUses) if (usableRank(use.rankId) != nullptr) { shift = use.increment; break; }
                borrowed.clear();
                borrowed[folderOwner][shift] = divisionKeys;
                rankFolders.clear();
                rankIds.clear();
                result.notes.add("\"" + stop.name + "\" plays the sample folders of \"" + stops.at(folderOwner).name + "\" (one folder, two ranks of the definition): loaded once, made a transmission");
            }
            else for (const auto& folder : rankFolders) ownerOfFolder.emplace(folder.getFullPathName().toLowerCase(), id);
        }
        if (rankFolders.empty() && !borrowed.empty())
        {
            int bestOwner = 0, bestShift = 0, bestKeys = -1;
            for (const auto& [owner, shifts] : borrowed)
                for (const auto& [shift, keys] : shifts)
                    if (stops.count(owner) > 0 && keys > bestKeys) { bestOwner = owner; bestShift = shift; bestKeys = keys; }
            if (bestOwner > 0)
            {
                PlanStop extension;
                extension.stopId = id; extension.name = StopNames::capitaliseAfterDivisionPrefix(stop.name);
                extension.extensionOfStopId = bestOwner;
                extension.shift = juce::jlimit(-24, 24, bestShift);
                // The keys of its borrowed rows (0.8.159: a treble-only stop borrowing a
                // bass stop's rank plays its own keys alone).
                for (const auto& use : stop.rankUses)
                {
                    if (usableRank(use.rankId) == nullptr) continue;
                    const int first = use.firstNote > 0 ? use.firstNote : k.lowNote;
                    const int last = use.count > 0 ? first + use.count - 1 : k.highNote;
                    extension.lowKey = extension.lowKey < 0 ? first : juce::jmin(extension.lowKey, first);
                    extension.highKey = extension.highKey < 0 ? last : juce::jmax(extension.highKey, last);
                }
                division.stops.push_back(extension);
                plannedStopIds.insert(id);
                ++result.extensions;
                juce::String note = "\"" + stop.name + "\" plays the ranks of \"" + stops.at(bestOwner).name + "\""
                    + (extension.shift != 0 ? " shifted by " + juce::String(extension.shift) + " semitones" : juce::String()) + ": made a transmission of it";
                int others = 0;
                for (const auto& [owner, shifts] : borrowed) others += (int)shifts.size();
                if (others > 1) note << " (its other " << (others - 1) << " borrowed piece(s) are covered by the filled pipes)";
                result.notes.add(note);
                continue;
            }
        }
        if (!borrowed.empty())
        {
            juce::StringArray from;
            for (const auto& [owner, shifts] : borrowed) if (stops.count(owner) > 0) from.add(stops.at(owner).name);
            result.notes.add("\"" + stop.name + "\" has ranks of its own and borrows keys of " + from.joinIntoString(", ") + " in Hauptwerk: its own ranks are loaded, the filled pipes cover the rest");
        }
        if (rankFolders.empty()) { ++result.stopsWithoutSamples; result.notes.add("\"" + stop.name + "\" has no usable pipes (tremmed, noise or missing samples): left out"); continue; }
        PlanStop planStop;
        planStop.stopId = id; planStop.name = StopNames::capitaliseAfterDivisionPrefix(stop.name); planStop.rankIds = rankIds;
        // The files' numbers against the keys, from the stop's ranks (0.8.155).
        auto sampleOffsetFor = [this](const std::vector<int>& ids)
        {
            std::map<int, int> weight;
            for (const int rankId : ids)
                if (const auto rank = ranks.find(rankId); rank != ranks.end()) weight[rank->second.sampleOffset] += (int)rank->second.pipes.size();
            int best = 0, bestWeight = -1;
            for (const auto& [offset, w] : weight) if (w > bestWeight) { best = offset; bestWeight = w; }
            return bestWeight > 0 ? best : 0;
        };
        planStop.sampleOffset = sampleOffsetFor(rankIds);
        if (planStop.sampleOffset != 0) ++offsetStops;
        // The keys of the stop's OWN rows for these ranks (0.8.159): a rank folder
        // shared by a bass stop and a treble stop keeps every file, each stop
        // plays its own keys (the others are switched off in its voicing).
        auto keyRangeFor = [&](const std::vector<int>& ids, int& low, int& high)
        {
            low = -1; high = -1;
            for (const auto& use : stop.rankUses)
            {
                if (use.increment != 0 || std::find(ids.begin(), ids.end(), use.rankId) == ids.end()) continue;
                const int first = use.firstNote > 0 ? use.firstNote : k.lowNote;
                const int last = use.count > 0 ? first + use.count - 1 : k.highNote;
                low = low < 0 ? first : juce::jmin(low, first);
                high = high < 0 ? last : juce::jmax(high, last);
            }
        };
        keyRangeFor(rankIds, planStop.lowKey, planStop.highKey);
        // Several ranks in ONE perspective (Caen: a stereo stop recorded as
        // an "(L component)" and an "(R component)" rank, two folders for the
        // same perspective): LibreOrgan Sampler's stop has one folder per
        // perspective, so the stop becomes a stop folder whose children are
        // the rank groups - the ranks matched across the perspectives by
        // their name without the perspective head ("Rec: Quintaton 16 (L
        // component)" in Front and Rear is one child), else by their folder
        // name. One rank per perspective stays one stop, as ever.
        {
            std::map<juce::String, int> ranksPerLabel;
            bool severalInOne = false;
            for (const int rankId : rankIds)
            {
                const auto& rank = ranks.at(rankId);
                if (++ranksPerLabel[rank.perspective.isNotEmpty() ? rank.perspective : juce::String("default")] > 1) severalInOne = true;
            }
            if (severalInOne)
            {
                using Groups = std::vector<std::pair<juce::String, std::vector<int>>>;
                auto groupBy = [&](bool byName)
                {
                    Groups groups;
                    for (const int rankId : rankIds)
                    {
                        const auto& rank = ranks.at(rankId);
                        juce::String key;
                        if (byName)
                        {
                            key = rank.name;
                            if (rank.perspective.isNotEmpty() && key.contains(":")) key = key.fromFirstOccurrenceOf(":", false, false).trim();
                        }
                        if (key.isEmpty()) key = rank.folder.getFileName();
                        bool found = false;
                        for (auto& group : groups) if (group.first.equalsIgnoreCase(key)) { group.second.push_back(rankId); found = true; break; }
                        if (!found) groups.push_back({ key, { rankId } });
                    }
                    return groups;
                };
                Groups groups = groupBy(true);
                if (groups.size() < 2) groups = groupBy(false);
                if (groups.size() >= 2)
                {
                    // The children by name ("(L component)" before "(R component)"), not in the file's order.
                    std::stable_sort(groups.begin(), groups.end(), [](const auto& a, const auto& b) { return a.first.compareNatural(b.first) < 0; });
                    for (const auto& [key, ids] : groups)
                    {
                        PlanStop child;
                        child.stopId = id; child.name = StopNames::capitaliseAfterDivisionPrefix(key); child.rankIds = ids;
                        keyRangeFor(ids, child.lowKey, child.highKey);
                        child.folders = foldersFor(ids);
                        child.sampleOffset = sampleOffsetFor(ids);
                        planStop.children.push_back(child);
                    }
                    result.notes.add("\"" + stop.name + "\" has " + juce::String((int)rankIds.size()) + " ranks in " + juce::String((int)ranksPerLabel.size())
                        + " perspective(s): made a stop folder with " + juce::String((int)groups.size()) + " stops (" + groups.front().first + " ...)");
                }
            }
        }
        if (planStop.children.empty()) planStop.folders = foldersFor(rankIds);
        // The definition's only folders for a stop named "...TREM" (Billerbeck
        // "P. Grosse flute 32": Ped_flute32/Rea_Ped_flute32TREM and nothing
        // else): taken as Hauptwerk plays them, said in the notes - the row names
        // the folder explicitly, so the "trem" pattern does not apply to it.
        {
            bool allTremNamed = !planStop.folders.empty();
            for (const auto& f : planStop.folders) if (!f.getFileName().containsIgnoreCase("trem")) allTremNamed = false;
            if (allTremNamed) result.notes.add("\"" + stop.name + "\": the definition plays it from folders named TREM only (" + planStop.folders.front().getFileName() + ") - taken as Hauptwerk has it");
        }
        // A one-shot stop (no release samples in any of its ranks): its folder
        // names go to the One-shot Patterns.
        {
            bool oneShot = !rankIds.empty();
            for (const int rankId : rankIds) if (!ranks.at(rankId).oneShot) oneShot = false;
            if (oneShot)
            {
                std::vector<juce::File> leaves = planStop.folders;
                for (const auto& child : planStop.children) for (const auto& f : child.folders) leaves.push_back(f);
                for (const auto& f : leaves)
                    if (std::find(result.oneShotFolders.begin(), result.oneShotFolders.end(), f.getFileName()) == result.oneShotFolders.end()) result.oneShotFolders.push_back(f.getFileName());
                result.notes.add("\"" + stop.name + "\" has no release samples: a one-shot stop (One-shot Patterns)");
            }
        }
        division.stops.push_back(planStop);
        plannedStopIds.insert(id);
    }
    // An extension of a stop that is not in the plan (left out, or in a division
    // beyond the eight) cannot be made.
    for (auto& division : result.divisions)
        for (size_t i = 0; i < division.stops.size();)
        {
            auto& stop = division.stops[i];
            if (stop.extensionOfStopId > 0 && plannedStopIds.count(stop.extensionOfStopId) == 0)
            {
                result.notes.add("\"" + stop.name + "\": the stop it plays the ranks of is not in the plan - left out");
                --result.extensions; ++result.stopsWithoutSamples;
                division.stops.erase(division.stops.begin() + (long)i);
                continue;
            }
            ++i;
        }
    // No stop folder may hold another stop's folder: the loader would read the
    // inner stop twice (once as a perspective folder of the outer one). Such a
    // stop takes its rank folders as stop folders of their own instead.
    {
        std::vector<juce::File> allFolders;
        for (auto& division : result.divisions) for (auto* stop : leavesOf(division)) for (const auto& f : stop->folders) allFolders.push_back(f);
        for (auto& division : result.divisions)
            for (auto* stop : leavesOf(division))
            {
                bool holdsAnother = false;
                for (const auto& f : stop->folders)
                    for (const auto& g : allFolders)
                        if (g != f && g.isAChildOf(f)) holdsAnother = true;
                if (!holdsAnother) continue;
                std::vector<juce::File> rankFolders;
                for (const int rankId : stop->rankIds)
                    if (const auto* rank = usableRank(rankId)) rankFolders.push_back(rank->folder);
                if (rankFolders.empty()) continue;
                result.notes.add("\"" + stop->name + "\": its folder " + stop->folders.front().getFileName() + " holds other stops - its rank folders are taken one by one");
                stop->folders = rankFolders;
            }
    }
    // The trees: the parents of the stop folders, the one most stops lie in first.
    for (size_t d = 0; d < result.divisions.size(); ++d)
    {
        auto& division = result.divisions[d];
        const auto& k = ordered[d];
        std::map<juce::String, int> treeCount;   // folder path (lower case) -> stops
        std::map<juce::String, juce::File> treeFile;
        for (const auto* stop : leavesOf(division))
            for (const auto& stopFolder : stop->folders)
            {
                const auto tree = stopFolder.getParentDirectory();
                const auto key = tree.getFullPathName().toLowerCase();
                ++treeCount[key];
                treeFile[key] = tree;
            }
        std::vector<std::pair<int, juce::File>> trees;
        for (const auto& [key, n] : treeCount) trees.push_back({ n, treeFile[key] });
        std::sort(trees.begin(), trees.end(), [](const auto& a, const auto& b) { return a.first != b.first ? a.first > b.first : a.second.getFullPathName() < b.second.getFullPathName(); });
        for (const auto& t : trees) division.trees.push_back(t.second);
        bool anyFolders = false;
        for (const auto* stop : leavesOf(division)) if (!stop->folders.empty()) anyFolders = true;
        if (division.trees.empty() && anyFolders) result.notes.add("division \"" + division.name + "\": no sample folders (its stops have no usable pipes)");
        else if (division.stops.empty() && k.divisionId > 0) result.notes.add("division \"" + division.name + "\" (keyboard \"" + k.name + "\") has no stops: it stays as the empty pedal");
    }
    for (const auto& division : result.divisions)
        for (const auto& tree : division.trees)
            if (std::find(result.allTrees.begin(), result.allTrees.end(), tree) == result.allTrees.end()) result.allTrees.push_back(tree);
    // The grid cells from the jamb page (0.8.142): within a division of ours, the
    // drawknobs' distinct x values are its columns and the distinct y values its
    // rows - the picture copied, relative to the division (the definitions put
    // the divisions in any order). A layout too wide or too tall to be ours
    // (more than 6 columns or 24 rows: several bands side by side) is left alone.
    if (!stopPositions.empty())
    {
        int placed = 0, skippedDivisions = 0;
        for (auto& division : result.divisions)
        {
            std::set<int> xs, ys;
            for (const auto& stop : division.stops)
                if (const auto p = stopPositions.find(stop.stopId); p != stopPositions.end()) { xs.insert(p->second.x); ys.insert(p->second.y); }
            if (xs.empty()) continue;
            if (xs.size() > 6 || ys.size() > 24) { ++skippedDivisions; continue; }
            for (auto& stop : division.stops)
            {
                const auto p = stopPositions.find(stop.stopId);
                if (p == stopPositions.end()) continue;
                stop.gridColumn = (int)std::distance(xs.begin(), xs.find(p->second.x));
                stop.gridRow = (int)std::distance(ys.begin(), ys.find(p->second.y));
                ++placed;
            }
        }
        if (placed > 0) result.notes.add("Grid: " + juce::String(placed) + " stop(s) placed as on the definition's page \"" + jambPageName + "\""
            + (skippedDivisions > 0 ? " (" + juce::String(skippedDivisions) + " division(s) left in the table's order: a layout too wide or too tall)" : juce::String()));
    }
    if (offsetStops > 0)
        result.notes.add("Files vs keys: " + juce::String(offsetStops) + " stop(s) have their sample files numbered off the keys they play on "
            "(the definition's pipe notes against the file names, e.g. the file 024 on key 36): the Stop List rows carry the offset and the "
            "loader puts every file on its key");
    // The couplers of the definition as couplers of ours (0.8.138): the keyboard
    // played = our source division, the keyboard or division pulled = our
    // target; the type from the semitones (0 unison, 12 the 4', -12 the 16') or
    // the melody flag. What our catalog has (MainComponent::rebuildCouplerCatalog):
    // a keyboard pulls a higher division in every type, itself at 16' and 4', I
    // from II (the usual exception) and the pedal a bass coupler; the rest
    // (Bellevue "Bom-Ped 8" of a floating division, a 24' coupler) is noted.
    {
        auto indexOfKeyboard = [&](int keyboardId) -> int
        {
            for (size_t i = 0; i < ordered.size(); ++i) if (ordered[i].id == keyboardId) return (int)i;
            return -1;
        };
        auto indexOfDivision = [&](int divisionId) -> int
        {
            for (size_t i = 0; i < ordered.size(); ++i)
            {
                const auto ids = divisionsOf(ordered[i]);
                if (std::find(ids.begin(), ids.end(), divisionId) != ids.end()) return (int)i;
            }
            return -1;
        };
        const int top = (int)ordered.size() - 1;
        for (const auto& c : couplers)
        {
            const int source = indexOfKeyboard(c.sourceKeyboardId);
            // A coupler to a division: the division of ours that holds its stops. The
            // stops of a floating division (Bellevue "Chamade") lie in several divisions
            // of ours by their names - nothing of ours to couple.
            bool floating = false;
            int target = -1;
            if (c.destKeyboardId > 0) target = indexOfKeyboard(c.destKeyboardId);
            else
            {
                std::set<int> holders;
                for (size_t i = 0; i < result.divisions.size(); ++i)
                    for (const auto& planStop : result.divisions[i].stops)
                        if (const auto it = stops.find(planStop.stopId); it != stops.end() && it->second.divisionId == c.destDivisionId) holders.insert((int)i);
                if (holders.size() == 1) target = *holders.begin();
                else if (holders.size() > 1) floating = true;
                else target = indexOfDivision(c.destDivisionId);   // no stop of it loaded: the keyboard playing it
            }
            juce::String type;
            if (c.unisonOff) type = "uoff";
            else if (c.melody) type = "mel";
            else if (c.bass) type = "bass";
            else if (c.shift == 0) type = "8";
            else if (c.shift == 12) type = "4";
            else if (c.shift == -12) type = "16";
            bool ok = source >= 0 && target >= 0 && source <= top && target <= top && type.isNotEmpty();
            if (ok)
            {
                if (target > source) ok = true;
                else if (target == source) ok = type == "16" || type == "4" || type == "uoff";
                else ok = (target >= 1 && target < source) || (target == 0 && type == "bass");   // the couplers downwards (I/III, II/III ...), 0.8.139; the pedal from a manual: its bass coupler (Doesburg "bass_1", 0.8.140)
            }
            juce::String label = c.switchName.isNotEmpty() ? c.switchName : c.name;
            for (const char* head : { "Slider Coupler ", "Slider ", "Coupler " })
                if (label.startsWithIgnoreCase(head)) label = label.substring((int)std::strlen(head)).trim();
            if (!ok)
            {
                ++result.couplersUnmapped;
                result.notes.add("coupler \"" + label + "\" (" + c.name + "): no coupler of ours for it"
                    + (floating ? juce::String(" - a floating division, its stops lie in several divisions of ours") : target < 0 ? juce::String(" - its division has no keyboard of ours") : type.isEmpty() ? " - " + juce::String(c.shift) + " semitones" : juce::String()));
                continue;
            }
            Plan::PlanCoupler pc;
            pc.id = juce::String(target) + "-" + juce::String(source) + "-" + type;
            pc.label = label;
            pc.switchId = c.switchId;
            bool known = false;
            for (const auto& existing : result.couplers) if (existing.id == pc.id) known = true;
            if (!known) result.couplers.push_back(pc);
        }
    }
    // What the whole definition holds besides (for the message): the tremmed
    // ranks (alternate ranks of the stops, never listed under them) and the noises.
    for (const auto& [id, rank] : ranks)
    {
        if (rank.tremmed) ++result.skippedTremmedRanks;
        else if (rank.noise) ++result.skippedNoiseRanks;
    }

    // The perspective patterns: for every perspective the rank names tell, the
    // common prefix of its rank folders' names under the stop folder
    // ("03-rearMk4-" of "03-rearMk4-UW_Quintadena8"), as LibreOrgan Sampler's own
    // "Name;prefix" lines. Only stops with two or more perspectives count.
    {
        // One perspective may carry several prefixes ("01-dirc-" in three
        // divisions, "03-dif2S-" in the fourth, both "Front A (direct)"): the
        // prefix of a rank folder is its name without the stop folder's name at
        // the end; every prefix seen twice or more joins the label's line
        // ("Direct;01-dirc-;03-dif2S-"), the commonest first.
        std::map<juce::String, std::map<juce::String, int>> prefixesByLabel;   // label -> prefix (as spelt first) -> rank folders
        std::map<juce::String, juce::String> spelling;                          // lower-case prefix -> first spelling
        for (auto& division : result.divisions)
            for (const auto* stopPtr : leavesOf(division))
            {
                const auto& stop = *stopPtr;
                if (stop.rankIds.size() < 2) continue;
                for (const int rankId : stop.rankIds)
                {
                    const auto& rank = ranks.at(rankId);
                    if (rank.perspective.isEmpty() || rank.folder == juce::File()) continue;
                    for (const auto& stopFolder : stop.folders)
                    {
                        // A FLAT layout (0.8.220, Bergen op Zoom: the rank folders lie directly in the
                        // division tree, "pipes/03-ortf_HW_Octava2" / "04-ab_" / "05-rea_", the stop's
                        // "folders" ARE the rank folders): the rank folder is the stop folder itself,
                        // the prefix comes from the stop's other rank folders beside it (the common
                        // ending rule below) - before, such stops were skipped and the patterns had no
                        // prefix, so a further perspective loaded later was never recognised.
                        const bool flat = rank.folder == stopFolder;
                        if (!flat && !rank.folder.isAChildOf(stopFolder)) continue;
                        auto below = rank.folder;
                        if (!flat) while (below.getParentDirectory() != stopFolder && below.getParentDirectory() != below) below = below.getParentDirectory();
                        const auto name = below.getFileName(), stopName = flat ? juce::String() : stopFolder.getFileName();
                        juce::String prefix;
                        if (!flat && name.length() > stopName.length() && name.endsWithIgnoreCase(stopName)) prefix = name.dropLastCharacters(stopName.length());
                        else
                        {
                            // The rank folder is not "<prefix><stop folder>" ("02-dir22-PED_Trombone_ATT_SE_SUMEM"
                            // under "PED_Trombone"): the prefix is what stands before the longest
                            // ending the stop's rank folder names share (0.8.132); failing that,
                            // the name up to and including its last separator run.
                            std::vector<juce::String> siblings;
                            for (const int otherId : stop.rankIds)
                            {
                                const auto other = ranks.find(otherId);
                                if (other != ranks.end() && other->second.folder != juce::File() && other->second.folder.getParentDirectory() == below.getParentDirectory())
                                    siblings.push_back(other->second.folder.getFileName());
                            }
                            int common = 0;
                            if (siblings.size() >= 2)
                                while (true)
                                {
                                    bool same = common < siblings.front().length();
                                    const auto c = same ? siblings.front()[siblings.front().length() - 1 - common] : 0;
                                    for (const auto& n : siblings) if (common >= n.length() || n[n.length() - 1 - common] != c) same = false;
                                    if (!same) break;
                                    ++common;
                                }
                            if (common >= 3 && name.length() > common)
                            {
                                prefix = name.dropLastCharacters(common);
                                // The separator that follows belongs to the prefix ("02-dir22-", "01-ab_").
                                for (int i = prefix.length(); i < name.length() && (name[i] == '-' || name[i] == '_' || name[i] == ' '); ++i) prefix << name[i];
                            }
                            else if (flat) prefix = juce::String();   // a flat rank with no sibling to compare: no guess from its own name
                            else
                            {
                                int cut = -1;
                                for (int i = 0; i < name.length(); ++i) if (name[i] == '-' || name[i] == '_' || name[i] == ' ') cut = i;
                                prefix = cut >= 2 ? name.substring(0, cut + 1) : juce::String();
                            }
                        }
                        if (prefix.length() < 3) break;   // a digit or two ("0-") names nothing: the loader learns those itself
                        const auto key = prefix.toLowerCase();
                        if (spelling.count(key) == 0) spelling[key] = prefix;
                        ++prefixesByLabel[rank.perspective][key];
                        break;
                    }
                }
            }
        for (const auto& [label, prefixes] : prefixesByLabel)
        {
            std::vector<std::pair<int, juce::String>> ordered;
            for (const auto& [key, n] : prefixes) if (n >= 2) ordered.push_back({ n, spelling[key] });
            if (ordered.empty()) continue;
            std::sort(ordered.begin(), ordered.end(), [](const auto& a, const auto& b) { return a.first != b.first ? a.first > b.first : a.second < b.second; });
            juce::String line = label;
            for (const auto& [n, prefix] : ordered) line << ";" << prefix;
            result.perspectivePatterns.push_back(line);
        }
    }
    // Where the configuration goes: OrganConfigurations OUTSIDE the Hauptwerk folders
    // (0.8.158: beside the folder that holds HauptwerkSampleSetsAndComponents -
    // "Hauptwerk", whatever its name - so nothing of ours lands inside Hauptwerk's
    // tree; 0.8.125-0.8.157 put it beside OrganDefinitions, before that into the
    // numbered package). The library root stays OrganInstallationPackages, reached
    // by a relative path (sampleLibraryRootRelative), so the file works from that
    // folder and from anywhere else the root is known. Archives of the organ lie
    // beside: the same OrganConfigurations serves the unpacked and the packed set.
    juce::String fileName = result.organName;
    for (const auto c : juce::String("\\/:*?\"<>|")) fileName = fileName.replaceCharacter(c, '-');
    result.configurationFile = configurationFolderFor(hauptwerkRoot).getChildFile(fileName.trim() + ".json");
    if (encryptedSamples > 0)
        result.error = juce::String(encryptedSamples) + " of " + juce::String(sampleCount) + " samples are encrypted (.hbw): Hauptwerk's encrypted sample sets cannot be played by LibreOrgan Sampler.";
    return result;
}

std::map<int, float> HauptwerkOdf::levelsForFolders(const std::vector<juce::File>& rankFolders) const
{
    std::map<int, float> sum;
    std::map<int, int> count;
    for (const auto& folder : rankFolders)
        for (const auto& [id, rank] : ranks)
        {
            if (rank.folder == juce::File() || rank.folder != folder) continue;
            for (const auto& pipe : rank.pipes) { sum[pipe.note] += pipe.levelDb; ++count[pipe.note]; }
        }
    std::map<int, float> result;
    for (const auto& [note, total] : sum) result[note] = total / (float)juce::jmax(1, count[note]);
    return result;
}

bool HauptwerkOdf::compassForFolders(const std::vector<juce::File>& rankFolders, int divisionLow, int divisionHigh, int& low, int& high) const
{
    // Every StopRank row that plays one of these rank folders without a
    // transposition: its keys are [first, first + count - 1]; a row without a
    // first key starts at the division's first key, one without a count runs
    // to the division's last. The union over the rows (a stop of several
    // pieces) is the compass. Nothing found: the whole compass.
    low = 128; high = -1;
    bool any = false;
    for (const auto& [id, stop] : stops)
        for (const auto& use : stop.rankUses)
        {
            if (use.increment != 0) continue;
            const auto rankIt = ranks.find(use.rankId);
            if (rankIt == ranks.end() || rankIt->second.folder == juce::File()) continue;
            bool matches = false;
            for (const auto& folder : rankFolders) if (folder == rankIt->second.folder) matches = true;
            if (!matches) continue;
            const int first = use.firstNote > 0 ? use.firstNote : divisionLow;
            const int last = use.count > 0 ? first + use.count - 1 : divisionHigh;
            low = juce::jmin(low, first); high = juce::jmax(high, last);
            any = true;
        }
    if (!any || high < low) return false;
    low = juce::jlimit(0, 127, low); high = juce::jlimit(0, 127, high);
    return true;
}
