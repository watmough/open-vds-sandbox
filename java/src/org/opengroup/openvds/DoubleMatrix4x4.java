package org.opengroup.openvds;
import org.opengroup.openvds.*;


public class DoubleMatrix4x4 extends ByteBufferBackedObject {

    public DoubleMatrix4x4() {
        this.createByteBuffer(4 * 4 * 8);
        this.set(
            (double)1, (double)0, (double)0, (double)0,
            (double)0, (double)1, (double)0, (double)0,
            (double)0, (double)0, (double)1, (double)0,
            (double)0, (double)0, (double)0, (double)1
            );
    }

    public DoubleMatrix4x4(DoubleMatrix4x4 rhs) {
        this.createByteBuffer(4 * 4 * 8);
        for (int i = 0; i < 4*4; ++i)
        {
            this.getByteBufferProxy().putDouble(i * 8, rhs.getByteBufferProxy().getDouble(i * 8));
        }
    }

    public DoubleMatrix4x4(DoubleVector4 x, DoubleVector4 y, DoubleVector4 z, DoubleVector4 t) {
        this.createByteBuffer(4 * 4 * 8);
        this.set(x, y, z, t);
    }

    public boolean equals(Object other) {
        if (other == this) return true;
        if (other == null) return false;
        if (getClass() != other.getClass()) return false;
        DoubleMatrix4x4 real_other = (DoubleMatrix4x4)other;
        return (this.getX().equals(real_other.getX()) &&
                this.getY().equals(real_other.getY()) &&
                this.getZ().equals(real_other.getZ()) &&
                this.getT().equals(real_other.getT()));
    }

    public void set(DoubleVector4 x, DoubleVector4 y, DoubleVector4 z, DoubleVector4 t) {
        this.setX(x);
        this.setY(y);
        this.setZ(z);
        this.setT(t);
    }

    public void set(double xx, double xy, double xz, double xt,
                    double yx, double yy, double yz, double yt,
                    double zx, double zy, double zz, double zt,
                    double tx, double ty, double tz, double tt
    ) {
        this.getByteBufferProxy().putDouble(0 * 8, xx);
        this.getByteBufferProxy().putDouble(1 * 8, xy);
        this.getByteBufferProxy().putDouble(2 * 8, xz);
        this.getByteBufferProxy().putDouble(3 * 8, xt);
        
        this.getByteBufferProxy().putDouble(4 * 8, yx);
        this.getByteBufferProxy().putDouble(5 * 8, yy);
        this.getByteBufferProxy().putDouble(6 * 8, yz);
        this.getByteBufferProxy().putDouble(7 * 8, yt);
        
        this.getByteBufferProxy().putDouble(8 * 8, zx);
        this.getByteBufferProxy().putDouble(9 * 8, zy);
        this.getByteBufferProxy().putDouble(10 * 8, zz);
        this.getByteBufferProxy().putDouble(11 * 8, zt);
        
        this.getByteBufferProxy().putDouble(12 * 8, tx);
        this.getByteBufferProxy().putDouble(13 * 8, ty);
        this.getByteBufferProxy().putDouble(14 * 8, tz);
        this.getByteBufferProxy().putDouble(15 * 8, tt);
    }

    public DoubleVector4 getX() {
        return new DoubleVector4(
            this.getByteBufferProxy().getDouble(0 * 8),
            this.getByteBufferProxy().getDouble(1 * 8),
            this.getByteBufferProxy().getDouble(2 * 8),
            this.getByteBufferProxy().getDouble(3 * 8));
    }

    public DoubleVector4 getY() {
        return new DoubleVector4(
            this.getByteBufferProxy().getDouble(4 * 8),
            this.getByteBufferProxy().getDouble(5 * 8),
            this.getByteBufferProxy().getDouble(6 * 8),
            this.getByteBufferProxy().getDouble(7 * 8));
    }

    public DoubleVector4 getZ() {
        return new DoubleVector4(
            this.getByteBufferProxy().getDouble(8 * 8),
            this.getByteBufferProxy().getDouble(9 * 8),
            this.getByteBufferProxy().getDouble(10 * 8),
            this.getByteBufferProxy().getDouble(11 * 8));
    }

    public DoubleVector4 getT() {
        return new DoubleVector4(
            this.getByteBufferProxy().getDouble(12 * 8),
            this.getByteBufferProxy().getDouble(13 * 8),
            this.getByteBufferProxy().getDouble(14 * 8),
            this.getByteBufferProxy().getDouble(15 * 8));
    }

    public void setX(DoubleVector4 value) {
        this.getByteBufferProxy().putDouble(0 * 8, value.getX());
        this.getByteBufferProxy().putDouble(1 * 8, value.getY());
        this.getByteBufferProxy().putDouble(2 * 8, value.getZ());
        this.getByteBufferProxy().putDouble(3 * 8, value.getT());
    }

    public void setY(DoubleVector4 value) {
        this.getByteBufferProxy().putDouble(4 * 8, value.getX());
        this.getByteBufferProxy().putDouble(5 * 8, value.getY());
        this.getByteBufferProxy().putDouble(6 * 8, value.getZ());
        this.getByteBufferProxy().putDouble(7 * 8, value.getT());
    }

    public void setZ(DoubleVector4 value) {
        this.getByteBufferProxy().putDouble(8 * 8, value.getX());
        this.getByteBufferProxy().putDouble(9 * 8, value.getY());
        this.getByteBufferProxy().putDouble(10 * 8, value.getZ());
        this.getByteBufferProxy().putDouble(11 * 8, value.getT());
    }

    public void setT(DoubleVector4 value) {
        this.getByteBufferProxy().putDouble(12 * 8, value.getX());
        this.getByteBufferProxy().putDouble(13 * 8, value.getY());
        this.getByteBufferProxy().putDouble(14 * 8, value.getZ());
        this.getByteBufferProxy().putDouble(15 * 8, value.getT());
    }

    public String toString() {
        String value = "(";
        for (int o = 0; o < 4; ++o)
        {
            if (o > 0)
                value = value + ", ";
            value = value + "(";
            for (int i = 0; i < 4; ++i)
            {
                if (i > 0)
                    value = value + ", ";
                value = value + this.getByteBufferProxy().getDouble(o * 4 * 8 + i * 8);
            }
            value = value + ")";
        }
        value = value + ")";
        return value;
    }
}
