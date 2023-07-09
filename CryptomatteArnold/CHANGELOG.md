### CryptomatteArnold 1.2.3 release notes: 

* Fixed deprecation warnings in Arnold 7.x (Thanks @cpichard and @ThiagoIze)
* Fixed Arnold-Python code for Arnold 7.x 
* Added tests that exercise rendering multiple times in one Arnold session

### CryptomatteArnold 1.2.2 release notes: 

* Fixed an issue where sidecar manifests did not work with Solaris (Thanks @cpichard)

### CryptomatteArnold 1.2.1 release notes: 

* Removed call to deprecated `AtCritSec` and replaced with `AtMutex` (Thanks @ThiagoIze)

### CryptomatteArnold 1.2.0 release notes: 

Adds capabilities required to use Cryptomatte in Solaris. (Note this doesn't automatically bring support for Cryptomatte to Solaris, but makes the necessary changes to CryptomatteArnold.)

* Added support for custom output drivers, needed for Solaris.  See #7
* Added ability to skip creating depth outputs automatically, needed for Solaris. See #7
* Support layer token in outputs. See #8
* Fixes tests (reblesses results) to work with Arnold 7
* Disables "compatibility maximizing" test as Cryptomatte is now included with Arnold

Thanks @sebastienblor

### CryptomatteArnold 1.1.2 release notes: 

* Removes calls to deprecated APIs and works with multiple Arnold universes. (Thanks @sebastienblor)

### CryptomatteArnold 1.1.1 release notes: 

* Fixed tests to work with Arnold 6 (were unnecessarily strict and were failing due to unrelated changes. 

### CryptomatteArnold 1.1 release notes: 

* Preview AOVs are now always displayed in display drivers, even when preview_in_exr is disabled. 
* Builds are supplied against Arnold 5.0.1.0 for Arnold 5.0.&ast and 5.1.&ast, and 5.2.0.0 for Arnold 5.2.&ast
* Python tests now return error code for failed tests

### CryptomatteArnold 1.0 release notes: 

* Renamed repo from AlShaders2 to CryptomatteArnold
* Fixed parallel node init issue where rarely Cryptomatte filter would not be properly set up
* Changed Windows requirement to Visual Studio 2015 redistributable
* Reorganized and cleaned up repo
* Moved documentation from wiki to /docs
* Tests for which OIIO is not required run without OIIO Python module present

### (AlShaders2) Beta 4 release notes: 

Bug fixes:
* Fixed crash with multiple Cryptomatte shaders in scene. 
* Added error detection for issue where Cryptomatte filters do not get set up correctly, (reported by one user by not reproducible). 

### (AlShaders2) Beta 3 release notes:

Features
* Support Arnold 5.1 adaptive sampling
* Support for mixed bit-depth EXR files
* Support long object names (up to 2048 characters)
* Added preview_in_exr control
  * False (default): Omit preview channels when EXR drivers are being used
  * True: Keep behavior as usual
* Support C4DtoA Arnold 5.1 naming changes
* Add controls to disable various kinds of name processing
* Switched to Daniel Schmidt's faster hash to float code

Bug fixes
* Crash when rendering with non-aov shaders in Arnold 5.1
* Skip disabled nodes in manifest

### (AlShaders2) Beta 2 release notes:

Features
* Test suite for developers
* Added Cinema 4D name parsing (Sen Haerens)
* aov_shader metadata for KtoA and MtoA
* Unicode and special characters in names are now tested and supported

Bug fixes
* Restored Maya name in special cases (ns1:obj1|ns2:obj2)
* Fixed some crashes with malformed names, added tests for this
* Fixed metadata when rendering multiple Cryptomattes into one EXR file (Sen Haerens)
* Fixed metadata with multiple drivers if one driver did not support metadata (Sen Haerens)
* Fixed manifests in custom Cryptomattes
* Fixed manifests for standard cryptomattes with array (per poly) overrides 
* Fixed special characters (quotes, slashes) in manifests
* Manifests no longer include list aggregate nodes (Sen Haerens)
* Fixed issue passing AtStrings to variadic arguments in gcc older than 4.8.x (Sachin Shrestha)
* Statically link the stdc++ lib (Sachin Shrestha)

### (AlShaders2) Beta 1 release notes:

* New AOV shader named simply "Cryptomatte" with all global Cryptomatte options
  * Removed Arnold Render Options user data
* Added sidecar manifests, which are deferred-written and work with deferred-loaded standins
* Support Integer Overrides
* Internal changes
  * Uses float AOVs for less overhead
  * Opacity now uses built in opacity AOV rather than storing on own samples
  * cryptomatte, cryptomatte_filter, and cryptomatte_manifest_driver are all packaged into one executable

Bug Fixes
* Fixed Multiple cameras
* Per-face shader assignments now work regardless of what shader is applied first (thanks to new AiShaderGlobalsGetShader() API from Solid Angle)
* Matte objects work properly (omitted from Cryptomattes)
* fixed issue with user cryptomattes behind transparency

