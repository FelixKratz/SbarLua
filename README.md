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

Install and update it with the command (this will build the lua module from
source and place it in `$USER/.local/share/sketchybar_lua/`):
```bash
(git clone https://github.com/FelixKratz/SbarLua.git /tmp/SbarLua && cd /tmp/SbarLua/ && make install && rm -rf /tmp/SbarLua/)
```

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

## Important Remarks
Calling shell functions using `os.execute` or `io.popen` should be avoided.
This is because these functions will block the entire lua event handler thread.
Instead use:
```lua
sbar.exec(<command>, [Optional: <lua_function>])
```
where the `<command>` can be any regular shell command. This function is truly
async, which means that the command is executed without blocking the event
thread. If you depend on the result of the `<command>` you can optionally
specify a function as a completion handler, which will receive the result of
the command as the first argument. Additionally, should the result have a JSON
structure, it will be parsed into a LUA table. E.g.:
```lua
sbar.exec("sleep 5 && echo TEST", function(result)
  print(result)
end)
```

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
local item = sbar.add(<"item", "space", "alias">, <optional: name>, <property_table>)
local bracket = sbar.add("bracket", <optional: name>, <member_table>, <property_table>)
local slider = sbar.add(<"slider", "graph">, <optional: name>, <width>, <property_table>)
local event = sbar.add("event", <name>, <optional: NSDistributedNotification>)
```
The `<name>` is the identifier of the item, if no identifier is specified, it
is generated automatically,
such that the `local item` can be used in the following to target this
item with further commands.

Depending on the type there might be additional arguments that can (or must)
be supplied e.g. if the `type` is `bracket`, the add command takes a list of
members as a third argument and the property table as the fourth argument. If
the `type` is `event` the `<property_table>` is replaced by an optional `NSDistributedNotificationCenter` notification name.

### Set Domain
```lua
item:set(<property_table>)
```

### Subscribe Domain
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

### Trigger Domain

```lua
sbar.trigger(<event>, <optional: env_table>)
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
Regularly the query command would result a JSON containing all the relevant information, here, however, this information is accessible as a LUA table and can be accessed as such, e.g.
```lua
local left_padding = front_app:query().icon.padding_left
```

### Push Domain
```lua
graph:push(<float_table>)
```
You can push values to a `graph` with the push domain. The `float` values of
the table should be between 0 and 1.

### Multiple Bars
If you are using muliple sketchybar instances, you can target the lua module to
interact with another sketchybar instance by providing the instance name right
after requiring the module like this:
```bash
sbar = require("sketchybar")
sbar.set_bar_name("bottom_bar")
```
where `bottom_bar` is an example bar name.

### JSON
```lua
local table = sbar.json_to_table(<json_string>)
```
where `<json_string>` is a string containing a JSON object. Functions provided
by the Sketchybar LUA API will automatically parse JSON strings into LUA tables,
but if you need to parse a JSON string into a LUA table manually, you can use
this function.
