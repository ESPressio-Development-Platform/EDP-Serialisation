# Composition and Providers

**Not applicable to the current implementation foundation.**

EDP-Serialisation currently declares no `EDP-System` Composition Domain, capability, provider Offer, Requirement, Property, Attribute, Bootstrap lifecycle, or provider-selection rule.

`EDP-System` is consumed for semantic Type/Field identity and schema traversal only. `EDP-BoundedTypes::TypeConversionAdapter` is an extension trait rather than an EDP-System Composition provider mechanism.

If later codec/Memory/Localisation implementation introduces provider substitution, this page must be updated in the same implementation slice with the exact cardinality, Properties/Attributes, ownership, and lifecycle rules.
