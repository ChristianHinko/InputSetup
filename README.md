# InputSetup
This is a C++ plugin extending EnhancedInput to improve workflows both in code and in editor. It makes use of Gameplay Tags to identify Input Actions via a configurable map in the project settings. The map is stored in an EngineSubsystem and is treated as a centralized place for all Input Actions. This removes the need for defining Input Actions in C++ and prevents code redundancy. Modules external to the game project can also contribute to the Input Actions map through a provided Primary Data Asset. Adding input actions is also supported for Dynamically loaded modules (e.g. Game Features).

Collaborators: Brian2524, ChristianHinko

Current engine version: 5.1.0
