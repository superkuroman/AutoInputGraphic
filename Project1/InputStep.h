#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <windows.h>

enum class MouseButton
{
    Left,
    Right,
    Middle,
};

struct ClickRecord
{
    int x;
    int y;
    MouseButton button;
    int delay_ms;
    double x_ratio;
    double y_ratio;
    bool has_position_ratio;
};

struct ScreenInfo
{
    int left;
    int top;
    int width;
    int height;
};

class InputStep
{
public:
    explicit InputStep(const char* outFile = "defined_clicks.json");
    void Update();

private:
    void SaveToFile(const char* filename);
    void PlaybackFromFile(const char* filename);
    void HandleRecord();
    void HandleSaveTrigger();
    void HandlePlaybackTrigger();
    void RecordClick(MouseButton button);
    void SendClick(MouseButton button);
    POINT ResolvePlaybackPoint(const ClickRecord& record) const;
    ScreenInfo GetCurrentScreenInfo() const;
    const char* ToString(MouseButton button) const;
    bool TryParseButton(const std::string& value, MouseButton* button) const;

    bool recording_;
    std::vector<ClickRecord> records_;
    ScreenInfo recording_screen_;
    bool lb_was_down_;
    bool rb_was_down_;
    bool mb_was_down_;
    std::chrono::steady_clock::time_point last_event_time_;

    bool y_was_down_;
    bool p_was_down_;
    bool f12_was_down_;

    std::string output_file_;
};
