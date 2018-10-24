# foo_timestamp_server
A foobar2000 component to start an HTTP server opening access to the current playback state and the timestamp.

Project in development

Version 0.8.0 (2018-10-24)

# A typical file structure
Dependent projects from the SDK are listed below:
- foobar2000
  - foo_timestamp_server (← the target project here)
  - ATLHelpers (from SDK)
  - foobar2000_component_client (from SDK)
  - helpers (from SDK)
  - SDK (from SDK)
  - shared (from SDK)
- pfc (from SDK)

# Boost required
The target project requires the following libraries from Boost:
- system
- date_time
- filesystem
- chrono
- thread
- regex

Make your own work to build them first.

**Note:** Specifically, the `mt-s-x32` version of these libraries are required.

# Why did I fail to build the project
Check out https://www.zybuluo.com/czyczk/note/1258776 for possible problems.

# API documentation
Planned.
