# UINavigation

UINavigation is a UE4 plugin that allows you to easily setup Navigation within UMG that works with Mouse, Keyboard, Gamepad, or any combination of those. It's designed to be hands-off, flexible and extendable. It has quite a few examples and a thorough documentation.

###Known issues

When the game is paused, if the player is using a UINavWidget, chaining navigation doesn't work out of the box (you'll have to modify the Engine's source code in order for this to work).
This happens due to the fact that timers run alogn with the World's DeltaTime (which remains the same when the game is paused).

## Documentation

![Documentation 1](/Screens/Docs_Overview.png)

![Documentation 2](/Screens/Docs_1.png)

![Documentation 3](/Screens/Docs_2.png)

![Documentation 4](/Screens/Docs_3.png)

![Options](/Screens/DefaultsPanel.png)

## Examples

Simple menus have minimal required setup:

![Example 1.1](/Screens/SimpleMenu_Designer.png)

![Example 1.2](/Screens/SimpleMenu_Graph.png)

Complex menus are manageable:

![Example 2.1](/Screens/OptionsMenu_Designer.png)

![Example 2.2](/Screens/OptionsMenu_Graph.png)


### Attribution

All item slots were made by yinakoSGA (https://opengameart.org/users/yinakosga) under the CC-BY 3.0 license (https://creativecommons.org/licenses/by/3.0/).

Font credits located in fonts folder.