local colors = require("colors")

sbar.bar({
  height = 45,
  color = colors.bar.bg,
  border_width = 2,
  border_color = colors.bar.border,
  shadow = false,
  sticky = true,
  padding_right = 10,
  padding_left = 10,
  y_offset = -8,
  margin = -2,
})
