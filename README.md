# Unreal-Engine-Software-Occlusion-Culling
This plugin is reintroducing the functionality of CPU-based occlusion culling after its removal from the Unreal Engine 5.

## Features
* **Standalone**: Not embedded in the core rendering pipeline, enabling customization and extension across all projects.
* **Legacy**: UE4 system stats and settings remain accessible for debugging and profiling.

## Installation

1. Clone or download the repository.
2. Copy the plugin to your Unreal Engine project's `Plugins` directory. If you don’t have one, create it.
3. Restart the Unreal Engine Editor.
4. Enable the `SoftwareOcclusionCulling` plugin via the Plugin Manager.

## Usage
1. You can adjust default occlusion settings in the Plugin Project Settings.
2. You can override defaul occlusion settings using the USoftwareOcclusionCullingOverride actor component.
3. You can debug bounds using `r.SoftwareOcclusionCulling.VisualizeBounds 1`

## Contributing

We welcome contributions! If you have a feature request, bug report, or want to improve the plugin, please open an issue or send a pull request.

## License

This project is released under the AGPL-3.0 license.

If you require a different license or have questions related to licensing, please reach out to adrian.popoviciu@katcodelabs.com.

## Credits

Created by Adrian-Marian Popoviciu. A special thanks to the Unreal Engine community for their valuable feedback and contributions.

