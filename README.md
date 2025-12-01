# StateSnap

Autosave plugin for [Orbiter Space Flight Simulator](http://orbit.medphys.ucl.ac.uk/).

Automatically saves your scenario at configurable intervals (1-60 minutes) based on real-world time. Saves are organized into `Scenarios/Autosave/<date>/` folders with filenames including the vessel name and simulation date/time.

## Installation

Extract the zip file and copy the `Modules` folder into your Orbiter installation directory. Then activate the plugin in the Orbiter Launchpad under Modules.

## Usage

Access the control dialog via **Ctrl+F4** → "StateSnap Control" to:
- Pause/resume autosave
- Adjust save interval
- Trigger immediate save
- Toggle on-screen notifications

## Building

This must be built as part of the Orbiter source tree. Copy to `Orbiter/Extern/StateSnap` and add `add_subdirectory(StateSnap)` to `Orbiter/Extern/CMakeLists.txt`.

```bash
cmake -B build -S .
cmake --build build --config Release --target StateSnap
```

## Credits

Originally created by [Thymo van Beers](https://github.com/ThymoNL/StateSnap) (2022).

This fork is maintained by [Lucio Franco](https://github.com/LucioFranco/orbiter-autosave).

## License

GPL-2.0 - See [LICENSE](LICENSE)
