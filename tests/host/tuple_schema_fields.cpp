#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <tuple>

#include <ESPressio_Serialisation.hpp>

namespace S=ESPressio::Serialisation;
namespace Sys=ESPressio::System;

struct TupleSchemaFixture final {
    inline static constexpr Sys::TypeIdentifier Identifier{
        Sys::TypeIdentifier::Storage{1U,0U,0U,0U,0U,0U,0U,77U}
    };
    std::tuple<std::uint16_t,bool,std::int8_t> ArgumentValues{};
    using Fields=Sys::FieldSet<
        Sys::FieldBinding<&TupleSchemaFixture::ArgumentValues,13U>>;
};
static_assert(S::SerialisableType<std::tuple<std::uint16_t,bool,std::int8_t>>);
static_assert(S::SerialisableType<std::tuple<>>);
static_assert(!S::SerialisableType<std::tuple<Sys::FieldIdentifier>>);
static_assert(S::SerialisableType<TupleSchemaFixture>);
static_assert(Sys::IdentifiedType<TupleSchemaFixture>);

template<class Codec>
void Verify(const TupleSchemaFixture& source) {
    std::array<std::byte,128U> data{};
    const auto measurement=S::Measure<Codec,S::RootProfile::TypedEnvelope>(source);
    assert(measurement.IsSuccessful());
    assert(measurement.RequiredBytes<=data.size());
    const auto result=S::Serialise<Codec,S::RootProfile::TypedEnvelope>(source,data);
    assert(result.IsSuccessful() && result.BytesWritten==measurement.RequiredBytes);
    TupleSchemaFixture decoded{};
    const auto round=S::Deserialise<Codec,S::RootProfile::TypedEnvelope>(
        std::span<const std::byte>(data.data(),result.BytesWritten),decoded);
    assert(round.IsSuccessful());
    assert(decoded.ArgumentValues==source.ArgumentValues);
    auto corrupt=data;
    if constexpr (std::is_same_v<Codec,S::Cbor>) {
        // CBOR tuple field must be array of exactly three elements.
        const std::array<std::byte,3U> marker{std::byte{0x0DU},std::byte{0x83U},
                                              std::byte{0x19U}};
        bool seen=false;
        for(std::size_t i=0;i+3U<=result.BytesWritten;++i) {
            if(corrupt[i]==marker[0] && corrupt[i+1]==marker[1] &&
               corrupt[i+2]==marker[2]) {
                corrupt[i+1]=std::byte{0x82U};
                seen=true;
                break;
            }
        }
        assert(seen);
        const auto invalid=S::Deserialise<Codec,S::RootProfile::TypedEnvelope>(
            std::span<const std::byte>(corrupt.data(),result.BytesWritten),decoded);
        assert(!invalid.IsSuccessful());
    } else {
        const std::string_view encoded{
            reinterpret_cast<const char*>(data.data()),result.BytesWritten};
        assert(encoded.find("\"13\":[300,true,-2]")!=std::string_view::npos);
    }
}

int main() {
    TupleSchemaFixture source{};
    source.ArgumentValues={std::uint16_t{300U},true,std::int8_t{-2}};
    Verify<S::Cbor>(source);
    Verify<S::Json>(source);

    // Same exact canonical array body as the existing positional owner.
    std::array<std::byte,32U> tupleCbor{},posCbor{};
    const auto a=S::Serialise<S::Cbor>(source.ArgumentValues,tupleCbor);
    const auto b=S::SerialisePositionalCbor<3U>(
        posCbor,std::uint16_t{300U},true,std::int8_t{-2});
    assert(a.IsSuccessful() && b.IsSuccessful() &&
           a.BytesWritten==b.BytesWritten);
    for(std::size_t i=0U;i<a.BytesWritten;++i)
        assert(tupleCbor[i]==posCbor[i]);

    // A generated message may legitimately have no positions at all.
    std::tuple<> empty{};
    std::array<std::byte,16U> out{};
    const auto e=S::Serialise<S::Cbor>(empty,out);
    assert(e.IsSuccessful() && e.BytesWritten==1U &&
           out[0]==std::byte{0x80U});
    const auto j=S::Serialise<S::Json>(empty,out);
    assert(j.IsSuccessful() && j.BytesWritten==2U &&
           out[0]==std::byte{'['} && out[1]==std::byte{']'});
}
