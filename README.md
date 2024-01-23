# Firedecor
An advanced window decoration plugin for the Wayfire window manager.

## Dependencies
- `wayfire` (duh)
- `librsvg`
- `boost`

## Installation
- Using the AUR:
  ```
  yay -S wayfire-firedecor-git
  ```
- Building from source:
  ```
  git clone https://github.com/AhoyISki/Firedecor
  cd Firedecor
  meson build
  meson compile -C build
  sudo meson install -C build
  ```

## Goals
- [x] Implement rounded corners;
- [x] Implement individual border sizes;
- [x] Implement app icons;
- [x] Implement title bars on any direction;
- [x] Implement completely modular decoration placement;
- [x] Implement individual corner rounding;
- [x] Multiple corner radii *Implemented as either the corner is there, or it is not;
- [x] Implement multiple themes.
- [x] Add accent colors to be used on anywhere on edges;
- [x] Implement a maximum title size with potentially animated title scrolling;
- [ ] Declare a minimum window size so the decorations can not look disgusting on small windows;
- [ ] Implement shadows.

## Configuration

<details><summary>Font options</summary>

- `font` will set what font will be used for titles. Default is `sans-serif`;
- `font_size` will set the font size, in pixels, for the title. Default is `21`;
- `active_title` will set the color for the font of active windows. Default is `\#1d1f21ff`;
- `inactive_title` will set the color for the font of inactive windows. Default is `\#1d1f21ff`;
- `max_title_size` will set a maximum title size, in pixels. If the title is bigger than this value, it will be capped so that the title, plus `...` can fit in the maximum title size. The default is `750`;

</details>

<details><summary>Border options</summary>

- `border_size` can take up to 4 parameters. If one is used, it will be the border size for every edge of the windows. If 2 parameters are used, the first one determines the size of the top edge, and the second one determines the size of the remaining edges. If 3 are used, the first one will be used for all edges. If 4 parameters are used, they are used in the following order: top, left, bottom, right. Default is `30 10`;
- `active_border` will set the color for the border of active windows. Default is `#1d1f21e6`;
- `inactive_border` will set the color for the border of inactive windows. Default is `#1d1f21e6`;
- `corner_radius` will set the radius for the corners of the windows. Use 0 for no radius. Default is `0`;

</details>

<details><summary>Outline options</summary>

- `outline_size` will set the size for the outline of the window. Default is 0;
- `active_outline` will set the color for the outline of active windows. Default is `#000000ff`;
- `inactive_outline` will set the color for the outline of inactive windows. Default is `#000000ff`;

</details>

<details><summary>Button options</summary>

- `button_size` will set the size of the buttons, in pixels. Default is 20;
- `button_style` is a string that sets the style used for the buttons. By default, there are 3 styles:
  - `wayfire`, witch is similar to the one used by wayfire by default;
  - `firedecor`, my own spin on a buttons style, with animated symbols that change in size, and a different maximize symbol;
  - `simple`, where the buttons have no symbols inside of them, they are simple circles.

  If you place anything else on this string, say, something like `my_theme`, you will have to provide `png`s or `svg`s so that the plugin can draw custom buttons. To accomplish that, do the following:
  1. Create the folder `~/.config/firedecor/button-styles/`;
  2. In it, create a folder with the name `my_theme`;
  3. Place figures for the buttons. They'll have to be called something like `type-status.png`, where `type` can be `close`, `minimize`, or `toggle-maximize`, and `status` can be `hovered`, `pressed`, or nothing. E.g. close.png, toggle-maximize-hover.png, minimize-pressed.png. Additionally, if `inactive_buttons` is set to `true`, you have to add a additional images with the `status` of `inactive`. You **Must** provide an image for each of the `type`s and `status`es listed above, so 9 images if `inactive_buttons == false`, and 12 images if `inactive_buttons == true`. The images can be equal to each other, if you don't want do differentiate between different `type`s or `status`es, just make sure that every entry is placed.
 - `normal_min`, `normal_max`, and `normal_close` set their respective button colors when the button isn't hovered. Default values are `#c89e2bff`, `#2ebb3aff`, and `#c24045ff`, respectively.
 - `hovered_min`, `hovered_max`, and `hovered_close` set their respective button colors when the button is hovered. Default values are `#ffe450ff`, `#60fc79ff`, and `#ff6572ff`, respectively.
 - `inactive_buttons` is a `bool` that tells the plugin to draw buttons differently, depending on them being in an active or an inactive window. Default is `false`; The default is `wayfire`;

</details>

<details><summary>Icon options</summary>

- `icon_size` sets the size for the icons, in pixels. Default is `20`;
- `icon_theme` sets the theme to be used for the icons, make sure that a folder exists on an appropriate position. Default is `hicolor`;

</details>

<details><summary>Accent options</summary>

- Accents are areas in the decoration's background that you want to be colored differently, they are placed in the layout, seen on the section below;
- `active_accent` sets the color for active accents. Default is `#f5f5f5ff`.
- `inactive_accent` sets the color for inactive accents. Default is `#e1dffeff`.

</details>

<details><summary>Layout options</summary>

- `layout` is a long string that determines where things should be placed on the edges of a window. Here's how it works:
  - Every symbol must be separated by a space;
  - The symbols `title`, `icon`, `maximize`, `minimize`, and `close`, will place their respective symbols on the window;
  - The symbol `p` will introduce a standardized padding, set by the `padding_size` option. The symbol `P` followed by a number, will place that many pixels of padding, for example, `P7` places 7 pixels of padding on the edge;
  - The symbol `|` changes where the symbols are being placed. Normally, they're on the left of the edge, if you place a `|`, they will be on the center, if you place another `|`, they will be placed on the right. Further `|`s will not change position;
  - The symbol `-` will change the edge the symbols are being placed in. By default, it will be the top edge, and every `-` will change the edge, counter-clockwise. In previous versions of `wayfire-firedecor`, you needed to end the layout with `-`, that is no longer the case.
  - The symbol `a` will initiate/end an accented area, it will start one if there wasn't one already, and it will end one if there was. You can more precisely position accents by using paddings, for example `a P5 title P5 a` will place a padding between each end of the accent, giving some space for the title. All corners will be rounded with this option.
  - The symbol `A` is much like `a`, but it is followed by a spaceless string, which tells the program what should be done to the edges of the accent. The default behaviour is to create 2 flat edges, and the available options are:
    - Any of `br tr tl bl` will round the respective corner (`t`op and `b`ottom `l`eft and `r`ight). These can be placed in any order, e.g. `Abltr` will round the top right and bottom left corners.
    - `/` and `\\` (must be 2 backslashes) will create a diagonal ending on the respective edge. For example, `A\\/` will create a diagonal that looks like \ on the left edge, and / on the right. This is positioned in relation to the text direction, specifically, they rotate based on the edge they're on. This option will not work if one of the corners on a respective edge is rounded, e.g. `Atr//` will only diagonalize the left edge.
    - `!` is a flat edge. This is just used to skip diagonalization of the left edge, for example, `A!\\` will diagonalize the right edge but keep the left edge flat.

  The default layout is `P5 title | | minimize p maximize p close P5 -`. Here's what this means:
  1. Place a padding with 5 pixels of size, followed by title on the left;
  2. Move to the center, do nothing;
  3. Move to the right;
  4. Place a minimize button, followed by a toggle maximize button and a close button, all separated by a standardized padding;
  5. Place a padding with 5 pixels of size;
  6. Finish the top edge and move on to the left edge, do nothing there;

  Here's what this layout looks like:
  ![Default Layout](/assets/default-layout.png)
- `padding_size` determines the size used for `p` on `layout`. Default is `2`;

</details>

<details><summary>Other options</summary>

- `ignore_views` is of `criteria` type, and determines witch windows will be ignored for decorations. In the future, I plan on adding the ability to create multiple themes and use them selectively, for example, a light and dark theme.
- `debug_mode` turns the titles of windows into their respective `app_id`s, followed by the maximum pixel size of the current font, which often differs from the `font_size`. This is used when the plugin fails at finding the icon for an app, or if you want more precision in the positioning of the decorations. More in [App Icon Debugging](#app-icon-debugging). Default is `false`;
- `round_on` chooses which corners will be rounded. `tr` means top right, `tl` is top left, `bl` is bottom left, `br` is bottom right, and `all` is all of them, e.g. `tl br` will round the top left and bottom right corners. Default is `all`;

</details>

<details><summary>Extra theme options</summary>

- `extra_themes` will be the declaration of existance for any extra themes you want to use, e.g. `dark light discord`. If the theme is not in here, no windows will use it. The default is ``;
- When it comes to extra themes, the configuration section will look exactly like the regular `firedecor` section, except you won't have the `ignore_views` and `extra_themes` options, and will gain the `uses_if` option;
- `uses_if` is of `criteria` type, and will match all the windows that should use the theme of the current section. There is no default, so if it is not present, no window will use the theme;
- When declaring new themes, you don't need to use every single option on the list. If the option isn't present, the theme will simply use the value from the default `firedecor` theme section, so something like:
  ```ini
  [firedecor]
  border_size = 10 10 10 10

  title_color = 0.0 0.0 0.0 1.0

  extra_themes = white_title

  [white_title]
  uses_if = app_id is "kitty"

  title_color = 1.0 1.0 1.0 1.0
  ```
  Will change the `title_color` on views with `app_id is "kitty"`, but the `border_size` will stay at `10 10 10 10`.

</details>

### Example Configs

Here's what the default configuration would look like:
<details><summary>Default config</summary>

```ini
[firedecor]
font = sans-serif
font_size = 21
active_font = \#1d1f21ff
inactive_font = \#1d1f21ff

border_size = 35 10
active_border = \#1d1f21e6
inactive_border = \#1d1f21e6
corner_radius = 15

outline_size = 0
active_outline = \#000000ff
inactive_outline = \#000000ff

button_size = 18
button_style = simple
normal_min = \#c89e2bff
hovered_min = \#fac636ff
normal_max = \#2ebb3aff
hovered_max = \#39ea49ff
normal_close = \#c24045ff
hovered_close = \#f25056ff
inactive_buttons = false

icon_size = 20
icon_theme = hicolor

active_accent = \#f5f5f5ff
inactive_accent = \#e1dfe1ff

layout = a | icon P4 title | minimize p maximize p close p Atrtl -
padding_size = 8

ignore_views = none
debug_mode = false
```
![Default layout](/assets/default-layout.png)

</details>

It's supposed to give off a "macOS" feel. Paste this into your `wayfire.ini` and add firedecor to your active plugins to get started.

This is my own configuration, feel free to copy it if you want
<details><summary>My own config</summary>

```ini
[firedecor]
font = Clear Sans
active_title = \#c5c8c6ff
inactive_title = \#c5c8c6ff

active_accent = \#18171aff
inactive_accent = \#1d1f21ff

ignore_views = title contains "steam" | title contains "Steam"
extra_themes = Discord Firefox

[Discord]
uses_if = app_id is "discord"

active_border = \#202225ff
inactive_border = \#1d1f21ff
border_size = 35 0
round_on = tr tl

layout = | icon P4 title | minimize p maximize p close p

[Firefox]
uses_if = app_id is "firefox"

active_title = \#1d1f21ff
inactive_title = \#1d1f21ff

active_border = \#f0f0f4ff
inactive_border = \#e1dfe1ff
border_size = 35 0
round_on = tr tl

layout = | icon P4 title | minimize p maximize p close p
```
![Personal layout](/assets/personal-layout.png)

</details>

## Screenshots
Left side decoration:
![Left side decoration](/assets/left-side-decoration.png)
Using:
```ini
border_size = 10 30 10 10
layout = - P5 title | | minimize p maximize p close P5 -
```

???:
![Strange 1](/assets/strange-1.png)

?̷̛͈͐̃̈́̀̇́̑͛̓͋̌?̴̡̘̯͙̩̂̑̅̆̕?̶͍̣́̅̐̔͂̅͐̿͌͝:
![Strange 2](/assets/strange-2.png)
(very laggy)

### Some ideas for accents
- Selective accents:
![top left and right](/assets/top-left-and-right.png)
```ini
layout = a P7 icon p title P7 a | | a P7 minimize p maximize p close P7 a 
```
- Connecting accents:
![top and bottom wrap](/assets/top-and-bottom-wrap.png)
```ini
layout = a | icon p title | P7 minimize p maximize p close P7 Atrtl - a P80 Atl - a | | Ablbr - a P80 Abr
```
- Framed outline:
![framed](/assets/framed.png)
```ini
border_size = 10
layout = a P100 Atl!\\ | | a P100 Atr/ - a P90 A!\\ | | a P90 A/ - a P100 Abl!/ | | a P100 Abr\\ - a P90 A!\\ | | a P90 A/
```
- Partial outlines using the same color for accents and the background:
![partial outline](/assets/partial-outline.png)
```ini
layout = P7 icon p title p a | | A p minimize p maximize p close P7 - a | | A - a | | Ablbr - a | | A
```
- Scroll-like decoration using visible outlines with an invisible background:
![scroll](/assets/scroll.png)
```ini
layout = a | icon p title | minimize p maximize p close P7 a - - P5 a | | a P5
```
- Connecting outlines with accents for cool effects (I think this one looks really cool):
![accent outlines](/assets/accent-outlines.png)
```ini
layout = | a P15 icon p title P15 A\/ |
```

## App Icon Debugging
The plugin will automatically try to retrieve icons from the file system, in order to display them on `icon` symbols on your windows. It will first look for folders matching your `icon_theme`. If it doesn't find the icons there, it will look in the remaining folders (hicolor, adwaita, breeze, in that order). However, sometimes, it just fails, and even if there is an icon for said app, the app's `app_id` is too terrible to find a suitable image, e.g. Osu!lazer has an `app_id` of "dotnet", which is completely unusable.
If this ends up happening, the plugin will use a backup icon, provided by the plugin itself. But you also have the ability to manually set icons for your apps. Here's how:
1. Set `debug_mode` to true;
2. Open your app, this should tell you what its `app_id` is, if you have a `title` in `layout`;
3. Find the icon for this app, it can be anywhere in the computer, and can be either a `png` or an `svg` file;
4. Find the file `~/.local/share/firedecor_icons`, it should be automatically created by the plugin;
5. Find the line containing the `app_id`, it should look like `my_app_id /full/path/to/default/icon`;
6. Replace the path in that line with the one you found earlier;
7. Done!
