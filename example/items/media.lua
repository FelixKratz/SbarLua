local whitelist = { ["Spotify"] = true,
                    ["Music"] = true   };

local media = sbar.add("item", "media", {
  icon = { drawing = false },
  position = "center",
  updates = true,
})

sbar.subscribe(media, "media_change", function(env)
  if whitelist[env.INFO.app] then
    sbar.set(media, {
      drawing = (env.INFO.state == "playing") and true or false,
      label = env.INFO.title
    })
  end
end)
