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

## Production dependency: EDP-Localisation

**Kind:** mandatory direct API dependency.

LocalisedText production source consumes `LanguageIdentifierView`, `LocalisationContext`, `WritableTextView`, validation status/facts, forward `ResolveFieldName`, context-specific reverse `ResolveFieldIdentifier`, and complete `ResolveFieldIdentifierAcrossLanguages`. EDP-Localisation remains authoritative for canonical RFC5646 syntax, pack/provider validation, fallback traversal, supported-language enumeration and ambiguity proof. Serialisation retains only caller-supplied Resolver pointers/views for the duration of one operation and never owns a pack.

## Demo/build dependencies

Every PlatformIO demo project consumes the local `EDP-Serialisation` library plus explicit `EDP-System`, `EDP-BoundedTypes`, and `EDP-Localisation` dependencies because the public Serialisation umbrella includes Localisation-backed profile support. PlatformIO resolves Serialisation's direct Memory/Platform-Portable metadata and the remaining transitive graph. `library.properties` declares all five direct Serialisation dependencies in the `0.1.x` range.


## Dependency direction invariant

The direct ownership remains acyclic:

```text
EDP-System ------------+
EDP-BoundedTypes ------+
EDP-Memory ------------+--> EDP-Serialisation
EDP-Platform-Portable -+
EDP-Localisation ------+
```

Foundational owners must not depend back on EDP-Serialisation. Transport, Persistence, Security, Threading, Command, Event, State and application layers are consumers above this repository rather than dependencies below it.

Typed Envelope consumes `System::TypeIdentifier` through the already-direct `EDP-System` dependency. No additional dependency edge is introduced by the envelope checkpoint.
