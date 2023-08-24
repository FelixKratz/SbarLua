local icons = require("icons")
local colors = require("colors")

local popup_toggle = "sketchybar --set $NAME popup.drawing=toggle"

sbar.add("item", "apple.logo", {
  padding_right = 15,
  click_script = popup_toggle,
  icon = {
    string = icons.apple,
    font = {
      style = "Black",
      size = 16.0,
    },
    color = colors.green,
  },
  label = {
    drawing = false,
  },
  popup = {
    height=35
  }
})

local apple_prefs = sbar.add("item", "apple.prefs", {
  position = "popup.apple.logo",
  icon = icons.preferences,
  label = "Preferences",
})

sbar.subscribe(apple_prefs, "mouse.clicked", function(_)
  os.execute("open -a 'System Preferences'")
  sbar.set("apple.logo", { popup = { drawing = false } } )
end)
