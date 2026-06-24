#include "register_types.h"

#include "core/object/class_db.h"

// Core
#include "src/event/UIEventDispatcher.h"
#include "src/Controller.h"
#include "src/GObject.h"
#include "src/GComponent.h"
#include "src/GRoot.h"

// Widgets
#include "src/GImage.h"
#include "src/GGraph.h"
#include "src/GGroup.h"
#include "src/GLoader.h"
#include "src/GLoader3D.h"
#include "src/GMovieClip.h"

// Text
#include "src/GTextField.h"
#include "src/GRichTextField.h"
#include "src/GTextInput.h"

// Components
#include "src/GButton.h"
#include "src/GLabel.h"
#include "src/GList.h"
#include "src/GTree.h"
#include "src/GTreeNode.h"
#include "src/GComboBox.h"
#include "src/GProgressBar.h"
#include "src/GSlider.h"
#include "src/GScrollBar.h"
#include "src/Window.h"

// Display
#include "src/display/FUIContainer.h"
#include "src/display/FUIInput.h"
#include "src/display/FUILabel.h"
#include "src/display/FUIRichText.h"
#include "src/display/FUISprite.h"

// Utilities
#include "src/UIPackage.h"
#include "src/ScrollPane.h"
#include "src/Transition.h"
#include "src/GPopupMenu.h"

void initialize_fairygui_module(ModuleInitializationLevel p_level)
{
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        // Core
        GDREGISTER_CLASS(fairygui::UIEventDispatcher);
        GDREGISTER_CLASS(fairygui::GController);
        GDREGISTER_CLASS(fairygui::GObject);
        GDREGISTER_CLASS(fairygui::GComponent);
        GDREGISTER_CLASS(fairygui::GRoot);

        // Widgets
        GDREGISTER_CLASS(fairygui::GImage);
        GDREGISTER_CLASS(fairygui::GGraph);
        GDREGISTER_CLASS(fairygui::GGroup);
        GDREGISTER_CLASS(fairygui::GLoader);
        GDREGISTER_CLASS(fairygui::GLoader3D);
        GDREGISTER_CLASS(fairygui::GMovieClip);

        // Text
        GDREGISTER_ABSTRACT_CLASS(fairygui::GTextField);
        GDREGISTER_CLASS(fairygui::GBasicTextField);
        GDREGISTER_CLASS(fairygui::GRichTextField);
        GDREGISTER_CLASS(fairygui::GTextInput);

        // Components
        GDREGISTER_CLASS(fairygui::GButton);
        GDREGISTER_CLASS(fairygui::GLabel);
        GDREGISTER_CLASS(fairygui::GList);
        GDREGISTER_CLASS(fairygui::GTree);
        GDREGISTER_CLASS(fairygui::GTreeNode);
        GDREGISTER_CLASS(fairygui::GComboBox);
        GDREGISTER_CLASS(fairygui::GProgressBar);
        GDREGISTER_CLASS(fairygui::GSlider);
        GDREGISTER_CLASS(fairygui::GScrollBar);
        GDREGISTER_CLASS(fairygui::GWindow);

        // Display
        GDREGISTER_CLASS(fairygui::FUIContainer);
        GDREGISTER_CLASS(fairygui::FUIInnerContainer);
        GDREGISTER_CLASS(fairygui::FUIInput);
        GDREGISTER_CLASS(fairygui::FUILabel);
        GDREGISTER_CLASS(fairygui::FUIRichText);
        GDREGISTER_CLASS(fairygui::FUISprite);

        // Utilities
        GDREGISTER_CLASS(fairygui::UIPackage);
        // ScrollPane(GComponent*) has no default constructor
        // Transition(GComponent*) has no default constructor
        GDREGISTER_CLASS(fairygui::GPopupMenu);
    }
}

void uninitialize_fairygui_module(ModuleInitializationLevel p_level)
{
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        // No explicit cleanup needed for Godot classes
    }
}
