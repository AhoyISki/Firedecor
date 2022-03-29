# wayfire-firedecor
An advanced window decoration plugin for the wayfire window manager.

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
  git clone https://github.com/AhoyISki/wayfire-firedecor
  cd wayfire-firedecor
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
- [ ] multiple corner radii(radiuses?);
- [ ] Implement shadows.
- [ ] Implement multiple themes.

## Configuration

### Font
- `font` will determine what font will be used for titles. Default is `sans-serif`;
- `font_size` will determine the font size, in pixels, for the title. Default is `20`;
- `active_title` will determine the color for the font of active windows. Default is `1.0 1.0 1.0 1.0`;
- `inactive_title` will determine the color for the font of inactive windows. Default is `1.0 1.0 1.0 1.0`;

### Border
- `border_size` can take up to 4 parameters. If one is used, it will be the border size for every edge of the windows. If 2 parameters are used, the first one determines the size of the top edge, and the second one determines the size of the remaining edges. If 3 are used, the first one will be used for all edges. If 4 parameters are used, they are used in the following order: top, left, bottom, right. Default is `30 10`;
- `active_border` will determine the color for the border of active windows. Default is `0.13 0.13 0.13 0.67`;
- `inactive_border` will determine the color for the border of inactive windows. Default is `0.2 0.2 0.2 0.87`;
- `corner_radius` will determine the radius for the corners of the windows. Use 0 for no radius. Default is `0`;

### Outline
- `outline_size` will set the size for the outline of the window. Default is 0;
- `active_outline` will determine the color for the outline of active windows. Default is `0.0 0.0 0.0 1.0`;
- `inactive_outline` will determine the color for the outline of inactive windows. Default is `0.0 0.0 0.0 1.0`;

### Buttons
- `button_size` will determine the size of the buttons, in pixels. Default is 20;
- `inactive_buttons` is a `bool` that tells the plugin to draw buttons differently, depending on them being in an active or an inactive window. Default is `false`;
- `button_style` is a string that sets the style used for the buttons. By default, there are 3 styles:
  - `wayfire`, wicth is similar to the one used by wayfire by default;
  - `firedecor`, my own spin on a buttons style, with animated symbols that change in size, and a different maximize symbol;
  - `simple`, where the buttons have no symbols inside of them, they are simple circles.

  If you place anything else on this string, say, something like `my_theme`, you will have to provide `png`s so that the plugin can draw custom buttons. To accomplish that, do the following:
  1. Create the folder `~/.config/firedecor/button-styles/`;
  2. In it, create a folder with the name `my_theme`;
  3. Place figures for the buttons. They'll have to be called something like `type-status.png`, where `type` can be `close`, `minimize`, or `toggle-maximize`, and `status` can be `hovered`, `pressed`, or nothing. E.g. close.png, toggle-maximize-hover.png, minimize-pressed.png. Additionally, if `inactive_buttons` is set to `true`, you have to add a additional images with the `status` of `inactive`. You **Must** provide an image for each of the `type`s and `status`es listed above, so 9 images if `inactive_buttons == false`, and 12 images if `inactive_buttons == true`. The images can be equal to each other, if you don't want do differentiate between different `type`s or `status`es, just make sure that every entry is placed.
  The default is `wayfire`;

### Icons
- `icon_size` determined the size for the icons, in pixels. Default is `20`;
- `icon_theme` determines the theme to be used for the icons, make sure that a folder exists on an appropriate position. Default is `hicolor`;

### Layout
- `layout` is a long string that determines where things should be placed on the edges of a window. Here's how it works:
  - Every symbol must be separated by a space;
  - The symbols `title`, `icon`, `maximize`, `minimize`, and `close`, will place their respective symbols on the window;
  - The symbol `p` will introduce a standardized padding, set by the `padding_size` option. The symbol `P` followed by a number, will place that many pixels of padding, for example, `P7` places 7 pixels of padding on the edge;
  - The symbol `|` changes where the symbols are being placed. Normally, they're on the left of the edge, if you place a `|`, they will be on the center, if you place another `|`, they will be placed on the right. Further `|`s will not change position;
  - The symbol `-` will change the edge the symbols are being placed in. By default, it will be the top edge, and every `-` will change the edge, counter-clockwise. You **Must** end the layout definition with one `-`, even if you don't plan on using the following edge.

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

### Other
- `ignore_views` is of `criteria` type, and determines witch windows will be ignored for decorations. In the future, I plan on adding the ability to create multiple themes and use them selectively, for example, a light and dark theme.
- `debug_mode` turns the titles of windows into their respective `app_id`s. This is used when the plugin fails at finding the icon for an app. More in [App Icon Debugging](#app-icon-debugging). Default is `false`;

### Example Config
Here's what the default configuration would look like:
```ini
[firedecor]
font = sans-serif
font_size = 20
active_font = 1.0 1.0 1.0 1.0
inactive_font = 1.0 1.0 1.0 1.0

border_size = 30 10
active_border = 0.13 0.13 0.13 0.67
inactive_border = 0.2 0.2 0.2 0.87
corner_radius = 0

outline_size = 0
active_outline = 0.0 0.0 0.0 1.0
inactive_outline = 0.0 0.0 0.0 1.0

button_size = 0
inactive_buttons = false
button_style = wayfire

icon_size = 20
icon_theme = hicolor

layout = P5 title | | minimize p maximize p close P5 -
padding_size = 2

ignore_views = none
debug_mode = false
```
paste this into your `wayfire.ini` and add firedecor to your active plugins to get started.

## Screenshots
Left side decoration:
![Left side decoration](/assets/left-side-decoration.png)
Using:
```ini
border_size = 10 30 10 10
layout = - P5 title | | minimize p maximize p close P5 -
```

My personal layout:
![Personal layout](/assets/personal-layout.png)
```ini
border_size = 30 5
corner_radius = 15
layout = | icon title | maximize p minimize p close P7 -
```

???:
![Strange 1](/assets/strange-1.png)

?̷̛͈͐̃̈́̀̇́̑͛̓͋̌?̴̡̘̯͙̩̂̑̅̆̕?̶͍̣́̅̐̔͂̅͐̿͌͝:
![Strange 2](/assets/strange-2.png)
(very laggy)

## App Icon Debugging
The plugin will automatically try to retrieve icons from the filesystem, in order to display them on `icon` symbols on your windows. It will first look for folders matching your `icon_theme`. If it doesn't find the icons there, it will look in the remaining folders (hicolor, adwaita, breeze, in that order). However, sometimes, it just fails, and even if there is an icon for said app, the app's `app_id` is too terrible to find a suitable image, e.g. Osu!lazer has an `app_id` of "dotnet", which is completely unusable.
If this ends up happening, the plugin will use a backup icon, provided by the plugin itself. But you also have the ability to manually set icons for your apps. Here's how:
1. Set `debug_mode` to true;
2. Open your app, this should tell you what its `app_id` is, if you have a `title` in `layout`;
3. Find the icon for this app, it can be anywhere in the computer, and can be either a `png` or an `svg` file;
4. Find the file `~/.local/share/firedecor_icons`, it should be automatically created by the plugin;
5. Find the line containing the `app_id`, it should look like `my_app_id /full/path/to/default/icon`;
6. Replace the path in that line with the one you found earlier;
7. Done!
