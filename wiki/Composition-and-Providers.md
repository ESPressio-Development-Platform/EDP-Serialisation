# Composition and Providers

**No Serialisation-owned Composition Domain or provider selection is required by the current implementation.**

EDP-Serialisation currently declares no `EDP-System` Composition Domain, capability, provider Offer, Requirement, Property, Attribute, Bootstrap lifecycle, or provider-selection rule.

`EDP-System` is consumed for semantic Type/Field identity and schema traversal only. `EDP-BoundedTypes::TypeConversionAdapter` is an extension trait rather than an EDP-System Composition provider mechanism.

JSON caller-buffer writes select an EDP-Memory ByteOperations provider as a compile-time operation template parameter, defaulting to EDP-Platform-Portable; Serialisation retains no provider instance. LocalisedText operations receive a caller-owned EDP-Localisation Resolver explicitly by reference together with caller-owned context/scratch. Serialisation does not discover/select a Localisation provider, create a Composition Requirement for it, or retain a Resolver/PackSource beyond the call.

If a later codec introduces a Serialisation-owned Composition Domain or provider selection, this page must be updated in the same implementation slice with exact cardinality, Properties/Attributes, ownership, and lifecycle rules.
