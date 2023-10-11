# SketchyBar Lua Plugin
This project wraps the SketchyBar API in LUA. This is done by using kernel
level inter-process communication between SketchyBar and the LUA module, such
that we do not have to alter the source code of SketchyBar at all.
The project is in early development, if you want to see how it can be used have
a look at the example in the example folder, which is a fully working
sketchybar config in LUA which should work out of the box with the current
SketchyBar version.

Feel free to contribute to this repo.

## LUA Module
This code in this repository compiles into a lua module which can be
required from any lua script.

For LUA to actually find the module, it has to reside in a path included in the
lua cpath (TODO: Install module into the default lua cpath), e.g.:
```lua
package.cpath = package.cpath .. ";/Users/" .. os.getenv("USER") .. "/.local/share/sketchybar_lua/?.so"
```

The module can then be required in a lua script
```lua
local sbar = require("sketchybar")
```
and used to communicate with SketchyBar.

## LUA API
### Bar Domain
```lua
sbar.bar(<property_table>)
```
where the `<property_table>` is a lua table with key value pairs. The possible
key-value pairs are the same as stated in the sketchybar documentation.

The shell SketchyBar API uses dots to indicate sub-properties, e.g.
`icon.y_offset=10 icon.width=50`, in this LUA API all dots are substituted by
sub-tables, i.e. `icon = { y_offset = 10, width = 50 }`.

### Default Domain
```lua
sbar.default(<property_table>)
```

### Add Domain
```lua
local item = sbar.add(<type>, <optional: name>, <property_table>)
```
where the `<type>` is a string specifying which component to add,
e.g. "item", "alias", "space", ...

The `<name>` is the identifier of the item, if no identifier is specified, it
is generated automatically,
such that the `local item` can be used in the following to target this
item with further commands.

Depending on the type there might be additional arguments that can (or must)
be supplied e.g. if the `type` is `bracket`, the add command takes a list of
members as a third argument and the property table as the fourth argument.

### Set Domain
```lua
item:set(<property_table>)
```
or equivalently
```lua
sbar.set(<name>, <property_table>)
```

### Subscribe Domain
```lua
sbar.subscribe(<name>, <event(s)>, <lua_function>)
```
or equivalently
```lua
item:subscribe(<event(s)>, <lua_function>)
```
where all regular sketchybar events are supported. Events can be supplied as a
single string or alternatively as a table of events. The `<lua_function>` is
called when the event occurs and receives one argument, which contains the
typical sketchybar environment variables, e.g. 
```lua
front_app:subscribe("front_app_switched", function(env)
  front_app:set({ label = { string = env.INFO } })
end)
```
or equivalently:
```lua
sbar.subscribe(front_app, "front_app_switched", function(env)
  sbar.set(env.NAME, { label = { string = env.INFO } })
end)
```

### Animate Domain
```lua
sbar.animate(<curve>, <duration>, <lua_function>)
```
where the `<curve>` and `<duration>` arguments are equivalent to those found in
the sketchybar documentation.
The lua function given as a third argument shall now contain all commands
belonging to this animation, i.e. all `set` or `bar` commands that shall be
animated.

### Query Domain
```lua
local info = item:query()
```
or equivalently
```lua
local info = sbar.query(<name>)
```
Regularly the query command would result a JSON containing all the relevant information, here, however, this information is accessible as a LUA table and can be accessed as such, e.g.
```lua
local left_padding = front_app:query().icon.padding_left
```
