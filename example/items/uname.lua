local uname = sbar.add("item", {
  position = "right",
  icon = { drawing = false },
  label = ":: " .. os.getenv("USER") .. " ::"
})
