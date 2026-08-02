#include "./GbaTypes.h"

using namespace NS_GBA;

u8 OBJ::GetYCoord() const {
    return (attr0 & 0xFF);
}

bool OBJ::RotateScale() const {
    return ((attr0 >> 8) & 0x01) > 0;
}

bool OBJ::DoubleSize() const {
    if (RotateScale()) {
        return ((attr0 >> 9) & 0x01) > 0;
    }

    return false;
}

bool OBJ::Disabled() const {
    if (RotateScale()) return false;

    return ((attr0 >> 9) & 0x01) > 0;
}

ObjectMode OBJ::Mode() const {
    return ObjectMode((attr0 >> 10) & 0x03);
}

bool OBJ::Mosaic() const {
    return ((attr0 >> 12) & 0x01) > 0;
}

ObjectColors OBJ::Colors() const {
    return ObjectColors((attr0 >> 13) & 0x01);
}

ObjectShape OBJ::Shape() const {
    return ObjectShape((attr0 >> 14) & 0x03);
}

u16 OBJ::GetXCoord() const {
    return (attr1 & 0x01FF);
}

u8 OBJ::GetRotateScaleParam() const {
    if (RotateScale()) {
        return ((attr1 >> 9) & 0x1F);
    }

    return 0xFF;
}

bool OBJ::FlipX() const {
    if (RotateScale()) return false;

    return ((attr1 >> 12) & 0x01) > 0;
}

bool OBJ::FlipY() const {
    if (RotateScale()) return false;

    return ((attr1 >> 13) & 0x01) > 0;
}

ObjectSize OBJ::Size() const {
    u8 shapeVal = static_cast<u8>(Shape());

    // TODO: Test and confirm this
    return ObjectSize((shapeVal << 2) | ((attr1 >> 14) & 0x03));
}

u16 OBJ::Name() const {
    return (attr2 & 0x03FF);
}

u8 OBJ::PriorityToBG() const {
    return ((attr2 >> 10) & 0x03);
}

u8 OBJ::PaletteNumber() const {
    if (Colors() == ObjectColors::SINGLE_PALETTE) return 0;

    return ((attr2 >> 12) & 0x0F);
}