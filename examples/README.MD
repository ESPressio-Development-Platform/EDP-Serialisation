# EDP-Serialisation Examples

Examples are complete, developer-oriented usage paths rather than profile-isolated feature demonstrations. They are maintained as runnable source and must comply with the same V2 code-convention policy as library code.

- [`json-numeric-round-trip`](./json-numeric-round-trip/README.MD) — defines a schema, proves `SerialisableType`, populates bounded data, measures JSON, serialises into caller-owned span storage, transactionally deserialises the exact written range, and verifies the decoded value.

Feature-focused validation projects remain under [`../demos`](../demos/README.MD).
