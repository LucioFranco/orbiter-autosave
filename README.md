# StateSnap

Autosave plugin for [Orbiter Space Flight Simulator](http://orbit.medphys.ucl.ac.uk/).

Automatically saves your scenario at configurable intervals (1-60 minutes) based on real-world time. Saves are organized into `Scenarios/Autosave/<date>/` folders with filenames including the vessel name and simulation date/time.

## Installation

Copy `StateSnap.dll` to your Orbiter `Modules/Plugin/` directory and activate it in the Orbiter Launchpad.

## Usage

Access the control dialog via **Ctrl+F4** → "StateSnap Control" to:
- Pause/resume autosave
- Adjust save interval
- Trigger immediate save
- Toggle on-screen notifications

## Building

Requires the Orbiter SDK.

```bash
cmake -B build -S . -DSTATESNAP_STANDALONE=ON -DORBITER_SDK_DIR=<path-to-sdk>
cmake --build build --config Release
```

## License

GPL-2.0 - See [LICENSE](LICENSE)
