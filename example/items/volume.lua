local colors = require("colors")
local icons = require("icons")

local volume_slider = sbar.add("slider", "volume_slider", {
  position = "right",
  updates = true,
  label = { drawing = false },
  icon = { drawing = false },
  slider = {
    highlight_color = colors.blue,
    width = 100,
    background = {
      height = 6,
      corner_radius = 3,
      color = colors.bg2,
    },
    knob= {
      string = "􀀁",
      drawing = false,
    },
  },
})

local volume_icon = sbar.add("item", "volume_icon", {
  position = "right",
  icon = {
    string = icons.volume._100,
    width = 0,
    align = "left",
    color = colors.grey,
    font = {
      style = "Regular",
      size = 14.0,
    },
  },
  label = {
    width = 25,
    align = "left",
    font = {
      style = "Regular",
      size = 14.0,
    },
  },
})

sbar.subscribe(volume_slider, "mouse.clicked", function(env)
  os.execute("osascript -e 'set volume output volume " .. env["PERCENTAGE"] .. "'")
end)

sbar.subscribe(volume_slider, "volume_change", function(env)
  local volume = tonumber(env.INFO)
  local icon = icons.volume._0
  if volume > 60 then
    icon = icons.volume._100
  elseif volume > 30 then
    icon = icons.volume._66
  elseif volume > 10 then
    icon = icons.volume._33
  elseif volume > 0 then
    icon = icons.volume._10
  end

  sbar.set(volume_icon, { label = icon })
  sbar.set(volume_slider, { slider = { percentage = volume } })
end)

local function animate_slider_width(width)
  sbar.animate("tanh", 30.0, function()
    sbar.set(volume_slider, { slider = { width = width }})
  end)
end

sbar.subscribe(volume_icon, "mouse.clicked", function()
  -- TODO: sbar.query syntax and parsing
  if tonumber(sbar.query(volume_slider).slider.width) > 0 then
    animate_slider_width(0)
  else
    animate_slider_width(100)
  end
end)
