#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <functional>

#include "PluginProcessor.h"

//==============================================================================
//  Palette: sunny, loud, friendly.
//==============================================================================
namespace pfui
{
inline juce::Colour sun()    { return juce::Colour (0xffffd60a); }   // sunshine yellow  = your bass (after)
inline juce::Colour orange() { return juce::Colour (0xffff9f1c); }
inline juce::Colour pink()   { return juce::Colour (0xffff4fa3); }   // hot pink         = the kick
inline juce::Colour mint()   { return juce::Colour (0xff3dffb0); }   // adding together
inline juce::Colour coral()  { return juce::Colour (0xffff4d4d); }   // fighting each other
inline juce::Colour sky()    { return juce::Colour (0xff4cc9ff); }
inline juce::Colour ink()    { return juce::Colour (0xff140f24); }   // deep background
inline juce::Colour ink2()   { return juce::Colour (0xff241a3d); }   // panels
inline juce::Colour cream()  { return juce::Colour (0xfffff4d6); }   // text
}

//==============================================================================
class SunLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SunLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics&, juce::TextButton&,
                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

//==============================================================================
//  Full-window "how do I connect my kick?" card. Click anywhere to close it.
//==============================================================================
class HelpOverlay : public juce::Component
{
public:
    std::function<void()> onClose;
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override { if (onClose) onClose(); }
};

//==============================================================================
class PhaseForgeAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    explicit PhaseForgeAudioProcessorEditor (PhaseForgeAudioProcessor&);
    ~PhaseForgeAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAtt  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    static constexpr int   kScope   = PhaseForgeAudioProcessor::kRing;     // ~1 s of decimated audio
    static constexpr float kDesignW = 1040.0f, kDesignH = 700.0f;

    // ---- setup -------------------------------------------------------------------------
    void timerCallback() override;
    void styleButton (juce::TextButton&, juce::Colour base, juce::Colour onColour,
                      juce::Colour textOff, juce::Colour textOn);
    void setupKnob (juce::Slider&, juce::Colour accent, double defaultValue);
    void setProOpen (bool);
    void setShiftMs (double ms);
    void nudgeShift (double deltaMs);
    double currentShiftParam() const;
    void setParamValue (const char* id, float plain);
    void resetAll();
    void doMatch();
    struct Effective { double engineMs, appliedMs, polarity; };
    Effective effective() const;                    // what the audio engine is set to RIGHT NOW (read from the parameters)                                  // MAKE IT MATCH

    // ---- painting ----------------------------------------------------------------------
    void text (juce::Graphics&, const juce::String&, juce::Rectangle<float>, float size,
               juce::Colour, juce::Justification, bool bold = true) const;
    void drawPanel (juce::Graphics&, juce::Rectangle<float>, const juce::String& title, juce::Colour accent) const;
    void drawHeader (juce::Graphics&) const;
    void drawHero (juce::Graphics&) const;
    void drawOverlap (juce::Graphics&, juce::Rectangle<float>) const;
    void drawMeter (juce::Graphics&, juce::Rectangle<float>) const;
    void drawFace (juce::Graphics&, juce::Rectangle<float>, float match) const;
    void drawProPanel (juce::Graphics&) const;
    void drawSparkle (juce::Graphics&, float x, float y, float size, juce::Colour) const;

    // ---- view computation (timer thread = message thread) ------------------------------
    void updateView();

    PhaseForgeAudioProcessor& proc;
    SunLookAndFeel laf;
    HelpOverlay help;

    // controls --------------------------------------------------------------------------------
    juce::Slider shiftK, rotStagesK, rotFreqK, rotQK, xoverK, speedK, mixK, outK;
    juce::ToggleButton flipB { "FLIP" }, autoB { "AUTO-FOLLOW" }, holdB { "FREEZE" }, hearB { "HEAR KICK" }, followB { "FOLLOW PITCH" };
    juce::TextButton matchB { "MAKE IT MATCH" }, resetB { "RESET" }, helpB { "? HELP" }, proB { "MORE" },
                     resetKeyB { "RESET KEY" };
    std::array<juce::TextButton, 6> nudgeB;
    std::array<juce::TextButton, 4> zoomB;
    juce::ComboBox bandBox, keyModeBox, keyRootBox, keyScaleBox, presetBox;

    struct KnobInfo { juce::Slider* s; const char* name; };
    std::array<KnobInfo, 7> proKnobs {};

    std::unique_ptr<SliderAtt> shiftAtt, rotStagesAtt, rotFreqAtt, rotQAtt, xoverAtt, speedAtt, mixAtt, outAtt;
    std::unique_ptr<ButtonAtt> flipAtt, autoAtt, followAtt, hearAtt;
    std::unique_ptr<ComboAtt>  bandAtt, keyModeAtt, keyRootAtt, keyScaleAtt;

    // layout (computed in resized) --------------------------------------------------------------
    float scale = 1.0f;
    bool  proOpen = false;
    juce::Rectangle<float> headerR, heroR, proR, waveR, meterR, faceR, keyChipR;

    // state ---------------------------------------------------------------------------------------
    int  zoomIdx = 1;                   // index into kZoomMs
    static constexpr std::array<float, 4> kZoomMs { 12.0f, 25.0f, 50.0f, 100.0f };

    // animation
    struct Particle { float x, y, vx, vy, size, phase; };
    std::array<Particle, 48> particles {};
    float tAnim = 0.f, kickPulse = 0.f, bassGlow = 0.f, matchS = -1.f;
    juce::Random rng;

    // snapshots
    std::array<float, kScope> sBass {}, sKick {}, sAligned {};
    std::array<unsigned char, kScope> kickMask {};
    bool haveScope = false;
    pf::PreviewScratch previewScratch;

    // view (overlap)
    double viewStart = 0.0, viewSamples = 100.0;
    bool   haveTrigger = false;
    double triggerIdx = 0.0;
    float  pkK = 1e-6f, pkB = 1e-6f;
    bool   kickHeard = false, bassHeard = false;

    // correlation meter: kick vs. the bass AFTER your settings, judged only where the kick is sounding
    float corr = 0.0f, corrEarlier = 0.0f, corrLater = 0.0f;
    bool  corrValid = false;
    pf::OverlapBest best;               // best achievable (any delay, either polarity)
    int   searchCountdown = 0;

    // interaction
    juce::Point<float> mousePos { -1.0f, -1.0f };
    bool   dragging = false;
    float  lastDragX = 0.0f;

    // MAKE IT MATCH feedback
    // is the host actually running the plugin? (audio-block heartbeat)
    unsigned lastBlocks = 0;
    juce::uint32 lastBlockTime = 0;
    bool running = false;
    float kickLvl = 0.f, bassLvl = 0.f;

    juce::String matchMsg;
    juce::uint32 matchMsgStamp = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhaseForgeAudioProcessorEditor)
};
