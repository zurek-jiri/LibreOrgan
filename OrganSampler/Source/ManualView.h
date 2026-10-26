#pragma once

#include <JuceHeader.h>
#include <functional>
#include <vector>

// The text of the manual and of the licences, laid out block by block (0.8.213): one
// juce::TextLayout per paragraph, heading or code block, built once per width and
// font scale and painted only where visible. A juce::TextEditor held the text before
// and laid the WHOLE text out again at every insert - 2.7 s for the manual, so the
// first opening waited that long (0.8.212 made every later opening instant). The
// blocks and their layouts are prepared right after the program's start
// (MainComponent::ensureAboutWindow), so the manual opens at once; the font can be
// made larger or smaller by the reader (the scale, "A-" / "A+" and Ctrl + wheel).
class ManualView final : public juce::Component
{
public:
    struct Link { int start = 0, end = 0, chapter = -1; };
    struct Block
    {
        juce::String text;             // as shown (no markdown marks)
        int level = 0;                 // 0 paragraph, 1-3 heading, -1 code (monospaced), -2 licence line (monospaced, small)
        int chapter = -1;              // the chapter this heading opens
        std::vector<Link> links;
        juce::TextLayout layout;
        float y = 0.0f, height = 0.0f;
        float layoutWidth = -1.0f, layoutScale = 0.0f;
        bool layoutHighlighted = false;
    };
    struct Selection { int block = -1, start = 0, end = 0; bool isEmpty() const { return block < 0 || end <= start; } };

    std::function<void(int)> onLinkClicked;
    std::function<void(float)> onScaleWheel;   // Ctrl + wheel: +0.1 / -0.1 asked for

    static constexpr float kMargin = 6.0f;

    void setBlocks(std::vector<Block>* newBlocks)
    {
        blocks = newBlocks;
        highlightedBlock = -1;
        selection = {};
        layoutAll();
    }
    void setScale(float newScale)
    {
        newScale = juce::jlimit(0.6f, 2.5f, newScale);
        if (std::abs(newScale - scale) < 0.001f) return;
        scale = newScale;
        layoutAll();
    }
    float getScale() const { return scale; }
    void setHighlightColour(juce::Colour c) { highlightColour = c; }
    // The heading of the chapter read now, painted in the highlight colour.
    void setHighlightedBlock(int block)
    {
        if (block == highlightedBlock) return;
        highlightedBlock = block;
        layoutAll();   // only the two headings concerned are laid out again
    }
    void setSelection(Selection s) { selection = s; repaint(); }
    Selection getSelection() const { return selection; }
    float blockTop(int block) const
    {
        if (blocks == nullptr || block < 0 || block >= (int)blocks->size()) return 0.0f;
        return (*blocks)[(size_t)block].y;
    }
    int blockAt(float y) const
    {
        if (blocks == nullptr) return -1;
        for (int i = 0; i < (int)blocks->size(); ++i)
            if (y < (*blocks)[(size_t)i].y + (*blocks)[(size_t)i].height) return i;
        return (int)blocks->size() - 1;
    }
    // The rectangles of the selected characters (component coordinates), the first one first.
    std::vector<juce::Rectangle<float>> selectionRects() const
    {
        std::vector<juce::Rectangle<float>> rects;
        if (blocks == nullptr || selection.isEmpty() || selection.block >= (int)blocks->size()) return rects;
        const auto& b = (*blocks)[(size_t)selection.block];
        for (int l = 0; l < b.layout.getNumLines(); ++l)
        {
            const auto& line = b.layout.getLine(l);
            if (!line.stringRange.intersects({ selection.start, selection.end })) continue;
            juce::Range<float> xs;
            bool any = false;
            for (auto* run : line.runs)
            {
                for (int i = 0; i < run->glyphs.size(); ++i)
                {
                    const int character = run->stringRange.getStart() + i;
                    if (character < selection.start || character >= selection.end) continue;
                    const auto& glyph = run->glyphs.getReference(i);
                    const juce::Range<float> g(glyph.anchor.x, glyph.anchor.x + glyph.width);
                    xs = any ? xs.getUnionWith(g) : g;
                    any = true;
                }
            }
            if (!any) continue;
            const auto bounds = line.getLineBounds();
            rects.push_back({ kMargin + line.lineOrigin.x + xs.getStart(), b.y + bounds.getY(), xs.getLength(), bounds.getHeight() });
        }
        return rects;
    }
    // Lays the blocks out for a width before the component has one (the pre-build at
    // the start): the window's text area is this wide when it opens.
    void prepareForWidth(int width)
    {
        setSize(width, getHeight());
        layoutAll();
    }
    void resized() override { layoutAll(); }
    void paint(juce::Graphics& g) override
    {
        if (blocks == nullptr) return;
        const auto clip = g.getClipBounds().toFloat();
        for (const auto& rect : selectionRects())
        {
            g.setColour(highlightColour.withAlpha(0.45f));
            g.fillRect(rect.expanded(1.0f, 0.0f));
        }
        const float width = (float)getWidth() - 2.0f * kMargin;
        for (const auto& b : *blocks)
        {
            if (b.y + b.height < clip.getY()) continue;
            if (b.y > clip.getBottom()) break;
            b.layout.draw(g, { kMargin, b.y, width, b.height });
        }
    }
    void mouseMove(const juce::MouseEvent& e) override
    {
        setMouseCursor(linkAt(e.getPosition()) >= 0 ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
    }
    void mouseUp(const juce::MouseEvent& e) override
    {
        if (e.mouseWasDraggedSinceMouseDown() || e.mods.isPopupMenu()) return;
        const int chapter = linkAt(e.getPosition());
        if (chapter >= 0 && onLinkClicked) onLinkClicked(chapter);
    }
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        if (e.mods.isCtrlDown() || e.mods.isCommandDown())
        {
            if (onScaleWheel && std::abs(wheel.deltaY) > 0.0001f) onScaleWheel(wheel.deltaY > 0.0f ? 0.1f : -0.1f);
            return;
        }
        juce::Component::mouseWheelMove(e, wheel);   // the viewport scrolls
    }
    // The text of a block as laid out (the find looks through it).
    static juce::String plainOf(const Block& b) { return b.text; }

private:
    juce::Font fontFor(const Block& b) const
    {
        if (b.level == -1) return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 14.0f * scale, juce::Font::plain));
        if (b.level == -2) return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 13.0f * scale, juce::Font::plain));
        if (b.level > 0)
        {
            const float size = b.level == 1 ? 24.0f : b.level == 2 ? 20.0f : 17.0f;
            return juce::Font(juce::FontOptions(size * scale, juce::Font::bold));
        }
        return juce::Font(juce::FontOptions(16.0f * scale));
    }
    void layoutBlock(Block& b, float width, bool highlighted)
    {
        juce::AttributedString s;
        s.setWordWrap(juce::AttributedString::byWord);
        s.setJustification(juce::Justification::topLeft);
        s.setLineSpacing(b.level < 0 ? 0.0f : 2.0f * scale);
        const auto font = fontFor(b);
        const auto colour = highlighted ? highlightColour : juce::Colours::white;
        if (b.links.empty()) s.append(b.text, font, colour);
        else
        {
            const juce::Font linkFont(juce::FontOptions(font.getHeight(), juce::Font::underlined));
            int from = 0;
            for (const auto& link : b.links)
            {
                if (link.start > from) s.append(b.text.substring(from, link.start), font, colour);
                s.append(b.text.substring(link.start, link.end), linkFont, juce::Colours::lightskyblue);
                from = link.end;
            }
            if (from < b.text.length()) s.append(b.text.substring(from), font, colour);
        }
        b.layout.createLayout(s, width);
        b.height = juce::jmax(b.layout.getHeight(), font.getHeight());
        b.layoutWidth = width;
        b.layoutScale = scale;
        b.layoutHighlighted = highlighted;
    }
    void layoutAll()
    {
        if (blocks == nullptr) return;
        const float width = juce::jmax(40.0f, (float)getWidth() - 2.0f * kMargin);
        const float gap = 0.55f * 16.0f * scale;
        float y = kMargin;
        for (int i = 0; i < (int)blocks->size(); ++i)
        {
            auto& b = (*blocks)[(size_t)i];
            const bool highlighted = i == highlightedBlock;
            if (b.layoutWidth != width || b.layoutScale != scale || b.layoutHighlighted != highlighted) layoutBlock(b, width, highlighted);
            if (b.level > 0 && i > 0) y += gap * (b.level == 1 ? 1.5f : 0.8f);   // air above a heading
            b.y = y;
            y += b.height + (b.level == -2 ? 0.0f : gap);
        }
        const int height = (int)std::ceil(y + kMargin);
        if (height != getHeight()) setSize(getWidth(), height);
        repaint();
    }
    int linkAt(juce::Point<int> p) const
    {
        if (blocks == nullptr) return -1;
        const int index = blockAt((float)p.y);
        if (index < 0) return -1;
        const auto& b = (*blocks)[(size_t)index];
        if (b.links.empty() || (float)p.y < b.y) return -1;
        const juce::Point<float> local((float)p.x - kMargin, (float)p.y - b.y);
        for (int l = 0; l < b.layout.getNumLines(); ++l)
        {
            const auto& line = b.layout.getLine(l);
            if (!line.getLineBoundsY().contains(local.y)) continue;
            for (auto* run : line.runs)
            {
                if (run->glyphs.isEmpty()) continue;
                for (const auto& link : b.links)
                {
                    if (run->stringRange.getStart() < link.start || run->stringRange.getEnd() > link.end) continue;
                    const auto& first = run->glyphs.getReference(0);
                    const auto& last = run->glyphs.getReference(run->glyphs.size() - 1);
                    const juce::Range<float> xs(line.lineOrigin.x + first.anchor.x, line.lineOrigin.x + last.anchor.x + last.width);
                    if (xs.contains(local.x)) return link.chapter;
                }
            }
        }
        return -1;
    }

    std::vector<Block>* blocks = nullptr;
    float scale = 1.0f;
    int highlightedBlock = -1;
    Selection selection;
    juce::Colour highlightColour = juce::Colours::orange;
};
