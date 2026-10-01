# Dependency Contracts

## Production dependency: EDP-System

**Kind:** mandatory direct package/API dependency.

Serialisation consumes `TypeIdentifier`, `FieldIdentifier`, `TypeAuthorityIdentifier`, `TypeLocalIdentifier`, `SchemaType`, `FieldBinding`, `FieldSet`, `FieldValueOf`, and `ForEachField`. System remains the sole owner of semantic identity and schema metadata; Serialisation does not copy these into a registry.

`SystemIdentifierAdapters.hpp` adapts System value Types from the Serialisation side so EDP-System does not acquire a reverse dependency. The current Serialisation Type classifier uses a safe structural pre-gate before `System::SchemaType` because `EDP-System#10`'s substitution-safe predicate correction is implemented on `serialisation_prerequisites` rather than current System `main`.

## Production dependency: EDP-BoundedTypes

**Kind:** mandatory direct package/API dependency.

Serialisation encoding/decoding consumes bounded `String`, `Bytes`, and `Vector` families plus `TypeConversionAdapter`, `IsTypeConversionAvailable`, `HasTypeConversionSuccessPredicate`, and `IsTypeConversionSuccessful`. BoundedTypes remains authoritative for bounded storage/capacity and pairwise semantic conversion.

## Production dependency: EDP-Memory

**Kind:** mandatory direct contract dependency.

JSON decode adds no new direct dependency edge. `JsonBufferSink` validates the selected provider through `Memory::Detail::ByteOperationsProviderTraits` and delegates byte-range writes to the provider's `CopyBytes`. Serialisation therefore does not implement a parallel `memcpy`/hand-written raw-copy abstraction. The provider is compile-time selected and no provider pointer/object is retained by a sink.

## Production dependency: EDP-Platform-Portable

**Kind:** mandatory direct concrete-default dependency.

`Platform::Portable::Memory::ByteOperationsProvider` is the default compile-time provider for `Serialise`. It is stateless and nothrow default-constructible. The public operation exposes the provider as a template parameter, allowing another provider satisfying the EDP-Memory contract to be selected without runtime registry/state.

Platform-Portable itself depends on EDP-Platform and EDP-Memory. EDP-Memory depends on System, Platform, and BoundedTopology. These remain transitive unless Serialisation source begins consuming their contracts directly.

## Demo/build dependencies

PlatformIO demo projects consume the local `EDP-Serialisation` library plus explicit System/BoundedTypes Git dependencies; PlatformIO resolves Serialisation's direct Memory/Platform-Portable metadata and the remaining transitive graph. `library.properties` declares all four direct Serialisation dependencies in the `0.1.x` range.

## Deferred direct dependency

`EDP-Localisation` becomes direct when the LocalisedText Field profile actually calls forward/reverse presentation resolution. It must not be added merely because later architecture anticipates it.

## Dependency direction invariant

The direct ownership remains acyclic:

```text
EDP-System ------------+
EDP-BoundedTypes ------+
EDP-Memory ------------+--> EDP-Serialisation
EDP-Platform-Portable -+
```

Foundational owners must not depend back on EDP-Serialisation. Transport, Persistence, Security, Threading, Command, Event, State and application layers are consumers above this repository rather than dependencies below it.
