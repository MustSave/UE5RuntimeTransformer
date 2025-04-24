# UE5RuntimeTransformer
A Runtime Gizmo Transformer tool helps you translate/rotate/scale objects in runtime! Easily provide editing tools to your final product!

Showcase Video: https://www.youtube.com/watch?v=0zys_jv5zyk

This is a UE5 Plugin made using C++ and Blueprints in Unreal Engine 5.4

Plugin targeted for both :
- Users that want to customize absolutely everything, from how the Gizmo looks like, to how it behaves and how it interacts with different objects
- Users that just want to quickly implement a Gizmo System in their game without having to customize much!

# Information

Sample project: https://github.com/xyahh/UE4RuntimeTransformer_Example

Documentation:  https://rtt.xyah.games/

Contact: juan@xyah.games

Supported Platforms: Windows, MacOS

# Version 1.0 Features

- Plugin designed to work with both Actors and Components and their respective Local Spaces(e.g. moving Components in their Relative space instead of moving them in Actor Local Space)

- Translation, Rotation, Scaling Available for Single & Multiple Actors/Components

- World Space & Local Space are both available for Translation and Rotation. Scaling is restricted to only work in Local Space.

- Cloning selections (for example, by holding a button while dragging) is supported. Components maintain hierarchy. Component-only cloning + Actor cloning are both supported in single player.

- Destruction of Selected Actors/Components supported.

- Snapping is supported for all transformations. Translation and Rotations are snapped based on their delta value, while Scaling is snapped based on the absolute value.

- Most functionality can be overriden (in both Blueprints & C++) for custom additional logic.

- UFocusable Interface for specific objects that require specific logic when Focused(Selected), Unfocused (Unselected), and when there is a Delta Transform pending.

# Example Assets Included
- Post Process Material for Object Selection
- Example Gizmo Meshes to make your own personalized Gizmo
- Example Gizmo Materials
- Example Gizmo Child Blueprints for each Transformation

# Current Known Issues
- When Rotating and Scaling, the Gizmos shake just a little bit. This can go unnoticed but
still an issue that needs fixing.

# Next Steps
- Fix known issues
