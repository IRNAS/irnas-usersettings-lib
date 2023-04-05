# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)

## [Unreleased]

### Added

-    Flag to indicate if a setting was recently changed.
-    Functions to clear the changed recently flag.
-    Functions for iterating over settings.
-    Submodule for getting and setting user settings via JSON.

## [1.3.4] - 2023-02-16

### Fixed

-   Zephyr include statements for NCS 2.2.

## [1.3.3] - 2023-02-03

### Fixed

-   On-change callbacks not being called when restoring settings.

## [1.3.2] - 2023-02-01

### Added

-   Get default functions.
-   Restore single setting functions.

## [1.3.1] - 2023-01-30

### Changed

-   Increase internall buffer in USS.

## [1.3.0] - 2023-01-27

### Added

-   Binary encoding and protocol.
-   User Settings Bluetooth Service (USS).

## [1.2.0] - 2023-01-18

### Added

-   Getter functions for a setting's max length.
-   Getter functions for a setting's type.

### Changed

-   Setting default with the same value as the existing default value does not return an error any more.

## [1.1.0] - 2023-01-12

### Added

-   Tab completion with setting keys to shell commands.

## [1.0.2] - 2023-01-12

### Fixed

-   Occasional crash when setting a setting value.

## [1.0.1] - 2023-01-10

### Fixed

-   Shell command to restore defaults.

## [1.0.0] - 2023-01-10

### Added

-   User settings library.
-   Basic sample.
-   Callbacks sample.

[Unreleased]: https://github.com/IRNAS/irnas-usersettings-lib/compare/v1.3.4...HEAD

[1.3.4]: https://github.com/IRNAS/irnas-usersettings-lib/compare/v1.3.3...v1.3.4

[1.3.3]: https://github.com/IRNAS/irnas-usersettings-lib/compare/v1.3.2...v1.3.3

[1.3.2]: https://github.com/IRNAS/irnas-usersettings-lib/compare/v1.3.1...v1.3.2

[1.3.1]: https://github.com/IRNAS/irnas-usersettings-lib/compare/v1.3.0...v1.3.1

[1.3.0]: https://github.com/IRNAS/irnas-usersettings-lib/compare/v1.2.0...v1.3.0

[1.2.0]: https://github.com/IRNAS/irnas-usersettings-lib/compare/v1.1.0...v1.2.0

[1.1.0]: https://github.com/IRNAS/irnas-usersettings-lib/compare/v1.0.2...v1.1.0

[1.0.2]: https://github.com/IRNAS/irnas-usersettings-lib/compare/v1.0.1...v1.0.2

[1.0.1]: https://github.com/IRNAS/irnas-usersettings-lib/compare/v1.0.0...v1.0.1

[1.0.0]: https://github.com/IRNAS/irnas-usersettings-lib/compare/f41f9e534d9a60b2f5d3584abf2836f2bde2b3fc...v1.0.0
