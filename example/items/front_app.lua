local front_app = sbar.add("item", "front_app", {
  icon = {
    drawing = false
  },
  label = {
    font = {
      style = "Black",
      size = 12.0,
    }
  }
})

sbar.subscribe(front_app, "front_app_switched", function(env)
  sbar.set(env.NAME, {
    label = {
      string = env.INFO
    }
  })
end)
