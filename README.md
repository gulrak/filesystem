# Filesystem

This is a header-only single-file std::filesystem compatible helper library,
based on the C++17 specs, but implemented for C++11 or C++14 (so not 100%
conforming to the C++17 standard). It is currently tested on macOS 10.12, Windows 10,
and Ubuntu 18.04 but should work on other versions too, as long as you have a
C++11 compatible compiler.

*It could still use some polishing, test coverage is above 90%, I didn't benchmark
much yet, but I'll try to optimize some parts and refactor others. Feedback
is welcome.*


## Motivation

I'm often in need of filesystem functionality, mostly `fs::path`, but directory
access too, and when beginning to use C++11, I used that language update
to try to reduce my third-party dependencies. I could drop most of what
I used, but still missed some stuff that I started implementing for the
fun of it. Originally I based these helpers on my own coding- and naming
conventions. When C++17 was finalized, I wanted to use that interface,
but it took a while, to push myself to convert my classes.

The implementation is closely based on chapter 30.10 from the C++17 standard
and a draft close to that version is
[Working Draft N4687](https://github.com/cplusplus/draft/raw/master/papers/n4687.pdf).
It is from after the standardization of C++17 but it contains the latest filesystem
interface changes compared to the
[Working Draft N4659](https://github.com/cplusplus/draft/raw/master/papers/n4659.pdf).

I want to thank the people working on improving C++, I really liked how the language
evolved with C++11 and the following standards. Keep on the good work!


## Platforms

`ghc::filesystem` is developed on macOS but tested on Windows and Linux.
It should work on any of these with a C++11-capable compiler. I currently
don't have a BSD derivate besides macOS, so the preprocessor checks will
cry out if you try to use it there, but if there is demand, I can try to
help. Still, it shouldn't replace `std::filesystem` where full C++17 is
available, it doesn't try to be a "better" `std::filesystem`, just a drop-in
if you can't use it.

Tests are currently run with:

* macOS 10.12: XCode 9.2 (clang-900.0.39.2), GCC 8.1.0, Clang 7.0.0
* Windows 10: Visual Studio 2017 15.8.5, MingW GCC 5.3
* Linux: Ubuntu 18.04LTS GCC 7.3 & GCC 8.0.1


## Tests

The header comes with a set of unit-tests and uses [CMake](https://cmake.org/)
as a build tool and [Catch2](https://github.com/catchorg/Catch2) as test framework.

All tests agains this implementation should succeed, depending on your environment
it might be that there are some warnings, e.g. if you have no rights to create
Symlinks on Windows or at least the test thinks so, but these are just informative.

To build the tests from inside the project directory under macOS or Linux just:

```cpp
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

This generates `filesystem_test`, the binary that runs all tests.

If the default compiler is a GCC 8 or newer, or Clang 7 or newer, it
additionally builds a version of the test binary compiled against GCCs/Clangs
`std::filesystem` implementation, named `std_filesystem_test`
as an additional test of conformance. Ideally all tests should compile and
succeed with all filesystem implementations, but in reality, there are
some differences in behavior and might be issues in these implementations.


## Usage

As it is a header-only library, it should be enough to copy the header
into your project folder oder point your include path to this directory and
simply include the `filesystem.h` header.

Everything is in the namespace `ghc::filesystem`, so one way to use it only as
a fallback could be:

```cpp
#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include) && __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include "filesystem.h"
namespace fs = ghc::filesystem;
#endif
```

If you want to also use the `fstream` wrapper with `path` support as fallback,
you might use:

```cpp
#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include) && __has_include(<filesystem>)
#include <filesystem>
namespace fs {
using namespace std::filesystem;
using ifstream = std::ifstream;
using ofstream = std::ofstream;
using fstream = std::fstream;
}
#else
#include "filesystem.h"
namespace fs {
using namespace ghc::filesystem;
using ifstream = ghc::filesystem::ifstream;
using ofstream = ghc::filesystem::ofstream;
using fstream = ghc::filesystem::fstream;
} 
#endif
```

Now you have e.g. `fs::ofstream out(somePath);` and it is either the wrapper or
the C++17 `std::ofstream`.

Note, that on MSVC this detection only works starting from version 15.7 on and when setting
the `/Zc:__cplusplus` compile switch, as the compiler allways reports `199711L`
without that switch ([see](https://blogs.msdn.microsoft.com/vcblog/2018/04/09/msvc-now-correctly-reports-__cplusplus/)).

Be aware too, as a header-only library, it is not hiding the fact, that it
uses system includes, so they "pollute" your global namespace.

There is a version macro `GHC_FILESYSTEM_VERSION` defined in case future changes
might make it needed to react on the version, but I don't plan to break anything.
It's the version as decimal number `(major * 10000 + minor * 100 + patch)`.


## Documentation

There is almost no documentation in this release, as any `std::filesystem` documentation
would work, besides the few differences explained in the next section. So you might
head over to https://en.cppreference.com/w/cpp/filesystem for a description of
the components of this library.

The only additions to the standard are documented here:


### `ghc::filesystem::ifstream`, `ghc::filesystem::ofstream`, `ghc::filesystem::fstream`

These are simple wrappers around `std::ifstream`, `std::ofstream` and `std::fstream`.
They simply add an `open()` method and a constuctor with an `ghc::filesystem::path`
argument as the `fstream` variants in C++17 have them.

### `ghc::filesystem::u8arguments`

This is a helper class that currently checks for UTF-8 encoding on non-Windows platforms but on Windows it
fetches the command line arguments als Unicode strings from the OS with

```cpp
::CommandLineToArgvW(::GetCommandLineW(), &argc)
```

and then converts them to UTF-8, and replaces `argc` and `argv`. It is a guard-like
class that reverts its changes when going out of scope.

So basic usage is:

```cpp
namespace fs = ghc::filesystem;

int main(int argc, char* argv[])
{
    fs::u8arguments u8guard(argc, argv);
    if(u8guard.valid()) {
        std::cerr << "Bad encoding, needs UTF-8." << std::endl;
        exit(EXIT_FAILURE);
    }

    // now use argc/argv as usual, they have utf-8 enconding on windows
    // ...

    return 0;
}
```

That way `argv` is UTF-8 encoded as long as the scope from `main` is valid.


## Differences

As this implementation is based on existing code from my private helper
classes, it derived some constraints of it, leading to some differences
between this and the standard C++17 API.


### LWG Defects

This implementation has switchable behavior for the LWG defects
[#2935](http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2935) and
[#2937](http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2937).
The currently selected behavior is following
[#2937](http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2937) but
not following [#2935](http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2935),
as I feel it is a bug to report no error on a `create_directory()` or `create_directories()`
where a regular file of the same name prohibits the creation of a directory and forces
the user of those functions to double-check via `fs::is_directory` if it really worked.

### Not Implemented

Besides this still being work-in-progress, there are a few cases where
there will be no implementation in the close future:

```cpp
// methods in path:
path& operator+=(basic_string_view<value_type> x);
int compare(basic_string_view<value_type> s) const;
```

These are not implemented, as there is no `std::basic_string_view` available in
C++11 and I did want to keep this implementation self-contained and not
write a full C++17-upgrade for C++11.


### Differences in API

```cpp
filesystem::path::string_type
filesystem::path::value_type
```

In Windows, an implementation should use `std::wstring` and `wchar_t` as types used
for the native representation, but as I'm a big fan of the
["UTF-8 Everywhere" philosophy](https://utf8everywhere.org/), I decided
agains it for now. If you need to call some Windows API, use the W-variant
with the `path::wstring()` member
(e.g. `GetFileAttributesW(p.wstring().c_str())`). This gives you the
Unicode variant independant of the `UNICODE` macro and makes sharing code
between Windows, Linux and macOS easier.

```cpp
const path::string_type& path::native() const /*noexcept*/;
const path::value_type *path::c_str() const /*noexcept*/;
```

These two can not be `noexcept` with the current implementation. This due
to the fact, that internally path is working on the generic path version
only, and the getters need to do a conversion to native path format.

```cpp
const path::string_type& path::generic_string() const;
```

This returns a const reference, instead of a value, because it can. This
implementation uses the generic representation for internal workings, so
it's "free" to return that.


### Differences in Behavior

#### fs.path [(ref)](https://en.cppreference.com/w/cpp/filesystem/path)

As the complete inner mechanics of this implementation `fs::path` are working
on the generic format, it is the internal representation. So creating any mixed
slash `fs::path` object under Windows (e.g. with `"C:\foo/bar"`) will lead to a
unified path with `"C:\foo\bar"` via `native()` and `"C:/foo/bar"` via
`generic_string()` API.

Additionally this implementation follows the standards suggestion to handle
posix paths of the form `"//host/path"` and USC path on windows also as having
a root-name (e.g. `"//host"`). The GCC implementation didn't choose to do that
while testing on Ubuntu 18.04 and macOS with GCC 8.1.0 or Clang 7.0.0. This difference
will show as warnings under std::filesystem. This leads to a change in the
algorithm described in the standard for `operator/=(path& p)` where any path
`p` with `p.is_absolute()` will degrade to an assignment, while this implementation
has the exception where `*this == *this.root_name()` and `p == preferred_seperator`
a normal append will be done, to allow:

```cpp
fs::path p1 = "//host/foo/bar/file.txt";
fs::path p2;
for (auto p : p1) p2 /= p;
ASSERT(p1 == p2);
```

For all non-host-leading paths the behaviour will match the one described by
the standard.

#### fs.op.copy [(ref)](https://en.cppreference.com/w/cpp/filesystem/copy)

Then there is `fs::copy`. The tests in the suite fail partially with C++17 `std::filesystem`
on GCC/Clang. They complain about a copy call with `fs::copy_options::recursive` combined
with `fs::copy_options::create_symlinks` or `fs::copy_options::create_hard_links` if the
source is a directory. There is nothing in the standard that forbids this combination
and it is the only way to deep-copy a tree while only create links for the files.
There is [LWG #2682](https://wg21.cmeerw.net/lwg/issue2682) that supports this
interpretation, but the issue ignores the usefulness of the combination with recursive
and part of the justification for the proposed solution is "we did it so for almost two years".
But this makes `fs::copy` with `fs::copy_options::create_symlinks` or `fs::copy_options::create_hard_links`
just a more complicated syntax for the `fs::create_symlink` or `fs::create_hardlink` operation
and I don't want to believe, that this was the intention of the original writing.
As there is another issue related to copy, with a different take on the description,
I keep my version the way I read the description, as it is not contradicting the standard and useful. Let's see
what final solution the LWG comes up with in the end.


## Open Issues

### General Known Issues

There are still some methods that break the `noexcept` clause, some
are related to LWG defects, some are due to my implementation. I
work on fixing the later ones, and might in cases where there is no
way of implementing the feature without risk of an exception, break
conformance and remove the `noexcept`.

### Windows

#### Symbolic Links

As symbolic links on Windows, while being supported more or less since
Windows Vista (with some strict security constraints) and fully since some earlier
build of Windows 10, when "Developer Mode" is activated, are at time of writing
(2018) rarely used, still they are supported with this implementation.

#### Permissions

The Windows ACL permission feature translates badly to the POSIX permission
bit mask used in the interface of C++17 filesystem. The permissions returned
in the `file_status` are therefore currently synthesized for the `user`-level
and copied to the `group`- and `other`-level. There is still some potential
for more interaction with the Windows permission system, but currently setting
or reading permissions with this implementation will most certainly not lead
to the expected behavior.


## Release Notes

### v1.0.2 (wip)

* Updated catch2 to v2.4.0.
* Refactored `fs.op.permissions` test to work with all tested `std::filesystem`
  implementations (gcc, clang, msvc++).
* Added helper class `ghc::filesystem::u8arguments` as `argv` converter, to
  help follow the UTF-8 path on windows. Simply instantiate it with `argc` and
  `argv` and it will fetch the Unicode version of the command line and convert
  it to UTF-8. The destructor reverts the change.
* Added `examples` folder with hopefully some usefull example usage. Examples are
  tested (and build) with `ghc::filesystem` and C++17 `std::filesystem` when
  available.

### [v1.0.1](https://github.com/gulrak/filesystem/tree/v1.0.1)

* Bugfix: `ghc::filesystem::canonical` now sees empty path as non-existant and reports
  an error. Due to this `ghc::filesystem::weakly_canonical` now returns relative
  paths for non-existant argument paths. [(#1)](https://github.com/gulrak/filesystem/issues/1)
* Bugfix: `ghc::filesystem::remove_all` now also counts directories removed [(#2)](https://github.com/gulrak/filesystem/issues/2)
* Bugfix: `recursive_directory_iterator` tests didn't respect equality domain issues
  and dereferencable constraints, leading to fails on `std::filesystem` tests.
* Bugfix: Some `noexcept` tagged methods and functions could indirectly throw exceptions
  due to UFT-8 decoding issues.
* `std_filesystem_test` is now also generated if LLVM/clang 7.0.0 is found.


### [v1.0.0](https://github.com/gulrak/filesystem/tree/v1.0.0)

This was the first public release version. It implements the full range of
C++17 std::filesystem, as far as possible without other C++17 dependencies.

