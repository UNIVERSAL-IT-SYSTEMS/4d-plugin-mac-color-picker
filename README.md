# 4d-plugin-mac-color-picker
Alternative color picker for 4D on Mac.

About
---
You might have noticed that if you pass the RGB value returned from the built-in [Select RGB color](http://doc.4d.com/4Dv15/4D/15/Select-RGB-Color.301-2007529.en.html) command directly to [OBJECT SET RGB COLORS](http://doc.4d.com/4Dv15/4D/15/OBJECT-SET-RGB-COLORS.301-2006928.en.html), you get a slightly **darker** colour.

![](https://github.com/miyako/4d-plugin-mac-color-picker/blob/master/images/picker.png)

For example, the standard window background colour is #E7E7E7.

However, if you pass this value to a form object, or even reference it in SVG, it is rendered slightly darker on an LCD.

![](https://github.com/miyako/4d-plugin-mac-color-picker/blob/master/images/result.png)

c.f. http://stackoverflow.com/questions/14578759/wrong-color-in-interface-builder

This plugin returns the exact RGB needed on screen, if you pass ```Picker standard color space```.

Moreover, it lets you displayed the alpha channel tool if you pass ```Picker RGBA color```.

The alpha value is returned in the most significant byte, currently not used in 4D.
