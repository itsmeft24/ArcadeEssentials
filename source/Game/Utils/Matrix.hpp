#pragma once

struct Matrix3x3 {
    union {
        float m[3][3];
        float _data[9];
    };
};

struct Matrix4x4 {
    union {
        float m[4][4];
        float _data[16];
    };

    static Matrix4x4 FromRotTrans(const Matrix3x3* rot3x3, const Vector3* trans3) {
        Matrix4x4 transform{};

        transform._data[0] = rot3x3->_data[0];
        transform._data[1] = rot3x3->_data[1];
        transform._data[2] = rot3x3->_data[2];
        transform._data[3] = 0.0f;

        transform._data[4] = rot3x3->_data[3];
        transform._data[5] = rot3x3->_data[4];
        transform._data[6] = rot3x3->_data[5];
        transform._data[7] = 0.0f;

        transform._data[8] = rot3x3->_data[6];
        transform._data[9] = rot3x3->_data[7];
        transform._data[10] = rot3x3->_data[8];
        transform._data[11] = 0.0f;

        transform._data[12] = trans3->x;
        transform._data[13] = trans3->y;
        transform._data[14] = trans3->z;
        transform._data[15] = 1.0f;

        return transform;
    }
};