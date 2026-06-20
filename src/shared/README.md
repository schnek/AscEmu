# shared

## Introduction

The `shared` project is the common systems layer used by both the **world server** and the **logon server**.

It contains the low-level runtime infrastructure that both executables depend on: configuration loading, cryptography, database support, debugging, logging, networking, threading, platform abstraction, and reusable utility code.

This module exists to avoid duplicating core systems across multiple server binaries and to ensure that foundational behavior stays consistent throughout the project.

Only code that is required by **both** world and logon should be placed here.

---

## Purpose

The purpose of `shared` is to provide one central place for common infrastructure and support code.

Typical reasons for putting code into `shared`:

- it is needed by both the world and logon server
- it provides low-level runtime infrastructure
- it should behave identically across executables
- it abstracts platform- or compiler-specific behavior
- it is reusable support code, not feature-specific gameplay logic

In practice, `shared` is the layer that the higher-level server code builds on top of.

---

## Current structure

### `Config`
Shared configuration loading and parsing code.

This area is responsible for reading server configuration data and exposing it in a reusable way to both server components.

Typical responsibilities:
- loading configuration files
- parsing and validating settings
- exposing typed access to shared configuration values

### `Cryptography`
Shared cryptographic and authentication-related code.

This directory contains common low-level crypto support needed by the project, especially in login- and authentication-related flows.

Typical responsibilities:
- authentication helper types and constants
- low-level cryptographic primitives
- reusable secure protocol support code

### `Database`
Shared database infrastructure.

This contains the common database layer used across the project and serves as the base for database access, query handling, and shared backend integration.

Typical responsibilities:
- database connection and query support
- shared data access abstractions
- common backend and runtime database logic

### `Debugging`
Shared diagnostics and debugging infrastructure.

This is the common place for runtime diagnostics, assertions, debugging support, and crash investigation helpers.

Typical responsibilities:
- assertion and fatal-error handling
- crash reporting support
- debugging utilities
- stack trace and diagnostic infrastructure

This directory is especially important because it provides the shared tools used to investigate failures in both executables.

### `Exceptions`
Shared exception-related code.

This directory contains common exception helpers and types that are intended to be reused across multiple subsystems.

### `Logging`
Shared logging infrastructure.

This contains the common logging system used throughout the project.

Typical responsibilities:
- console and file logging
- common log formatting
- shared logging APIs and output paths

Using one shared logging layer keeps diagnostics consistent across all executables.

### `Network`
Shared low-level networking infrastructure.

This is the common base for socket handling and related network support code that should not be duplicated between projects.

Typical responsibilities:
- socket abstractions
- low-level network support code
- common protocol transport infrastructure
- shared networking runtime helpers

This layer is infrastructure-focused and should stay separate from high-level gameplay or application logic.

### `Platform`
Shared platform and compiler abstraction.

This contains the code needed to keep the project portable across operating systems and toolchains.

Typical responsibilities:
- platform detection
- compiler compatibility helpers
- symbol visibility and import/export definitions
- platform-specific support code kept behind shared interfaces

### `Registry`
Shared registration-style support code.

This directory is used for common registration patterns or globally accessible subsystem support where a registry-style approach is needed.

### `Threading`
Shared threading and synchronization infrastructure.

This contains the concurrency-related building blocks reused across the project.

Typical responsibilities:
- threading helpers
- synchronization support
- reusable concurrency infrastructure
- platform-safe thread-related abstractions

### `Utilities`
General reusable support code.

This directory is intended for small shared helpers that do not belong more naturally in a more specialized subsystem.

Typical responsibilities:
- general reusable helpers
- callback/support structures
- small low-level utility code

This directory should remain focused and should not become a generic dumping ground.

---

## Important top-level files

### `AuthCodes.hpp`
Shared authentication-related constants or definitions used across the project.

### `BuildInfo.hpp.in`
A generated build-information template used to expose build metadata such as version, branch, commit, platform, or configuration details.

### `LogonCommDefines.hpp`
Shared communication-related definitions for logon/world interaction or protocol-level shared constants.

### `WoWGuid.hpp`
A shared World of Warcraft GUID helper type.

This file represents a domain/protocol-specific identifier abstraction that is used across multiple parts of the project.

Typical responsibilities:
- GUID representation
- packed/unpacked GUID handling
- shared protocol-facing identifier support

### `AEVersion.hpp.in`
A build-generated version template used to expose version information to the project at compile time.

### `pchShared.hpp`
The precompiled header for the shared module.

This file exists to reduce build cost by centralizing commonly included headers for the shared project.

---

## Design rules

### Shared-only scope
`shared` should only contain code that is genuinely used by both the world and logon server.

Code that is specific to only one executable should stay in that executable’s own module.

### Infrastructure first
`shared` is primarily an infrastructure layer, not a gameplay layer.

It should contain foundational systems such as:
- configuration
- logging
- debugging
- database support
- cryptography
- threading
- networking infrastructure
- platform abstraction
- shared utility code

### Keep module boundaries clean
Subsystem-specific high-level logic should not be moved into `shared` just because it is convenient.

A good rule is:
- if it is reusable infrastructure, it likely belongs here
- if it is application or gameplay behavior, it likely does not
