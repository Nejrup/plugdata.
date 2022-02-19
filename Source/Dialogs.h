/*
 // Copyright (c) 2021-2022 Timothy Schoen
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#pragma once

#include <JuceHeader.h>

class SaveDialog;
class ArrayDialog;
class SettingsDialog;

struct Dialogs
{
    static void showSaveDialog(Component* centre, std::function<void(int)> callback);
    static void showArrayDialog(Component* centre, std::function<void(int, String, String)> callback);

    static std::unique_ptr<Component> createSettingsDialog(AudioProcessor& processor, AudioDeviceManager* manager, const ValueTree& settingsTree, const std::function<void()>& updatePaths);

    static void showObjectMenu(Component* parent, Component* target, const std::function<void(String)>& cb);
};
