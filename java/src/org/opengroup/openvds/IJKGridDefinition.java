/////////////////////////////////////////////////////////////////////////////
// <copyright>
// Copyright (c) 2016 Bluware Inc. All rights reserved.
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written permission of the copyright owner.
// </copyright>
/////////////////////////////////////////////////////////////////////////////

// This file is auto-generated
package org.opengroup.openvds;
import org.opengroup.openvds.*;

/**
 * Definition for a regular IJK grid
 */

public final class IJKGridDefinition {
    /**
     * The XYZ origin of the IJK grid (i.e. the XYZ position of IJK coordinate 0,0,0)
     */

    DoubleVector3 Origin;

    /**
     * The XYZ spacing of a single unit in the I direction
     */

    DoubleVector3 IUnitStep;

    /**
     * The XYZ spacing of a single unit in the J direction
     */

    DoubleVector3 JUnitStep;

    /**
     * The XYZ spacing of a single unit in the K direction
     */

    DoubleVector3 KUnitStep;

    public DoubleVector3 getOrigin() {
        return this.Origin;
    }

    public void setOrigin(DoubleVector3 value) {
        this.Origin = value;
    }

    public DoubleVector3 getIUnitStep() {
        return this.IUnitStep;
    }

    public void setIUnitStep(DoubleVector3 value) {
        this.IUnitStep = value;
    }

    public DoubleVector3 getJUnitStep() {
        return this.JUnitStep;
    }

    public void setJUnitStep(DoubleVector3 value) {
        this.JUnitStep = value;
    }

    public DoubleVector3 getKUnitStep() {
        return this.KUnitStep;
    }

    public void setKUnitStep(DoubleVector3 value) {
        this.KUnitStep = value;
    }

    public boolean equals(Object other) {
        if (other == this) return true;
        if (other == null) return false;
        if (getClass() != other.getClass()) return false;
        IJKGridDefinition real_other = (IJKGridDefinition)other;
        return (this.Origin.equals(real_other.Origin) &&
            this.IUnitStep.equals(real_other.IUnitStep) &&
            this.JUnitStep.equals(real_other.JUnitStep) &&
            this.KUnitStep.equals(real_other.KUnitStep));

    }

    public double getOriginX() {
        return this.getOrigin().getX();
    }

    public double getOriginY() {
        return this.getOrigin().getY();
    }

    public double getOriginZ() {
        return this.getOrigin().getZ();
    }

    public void setOriginX(double value) {
        this.getOrigin().setX(value);
    }

    public void setOriginY(double value) {
        this.getOrigin().setY(value);
    }

    public void setOriginZ(double value) {
        this.getOrigin().setZ(value);
    }

    public double getIUnitStepX() {
        return this.getIUnitStep().getX();
    }

    public double getIUnitStepY() {
        return this.getIUnitStep().getY();
    }

    public double getIUnitStepZ() {
        return this.getIUnitStep().getZ();
    }

    public void setIUnitStepX(double value) {
        this.getIUnitStep().setX(value);
    }

    public void setIUnitStepY(double value) {
        this.getIUnitStep().setY(value);
    }

    public void setIUnitStepZ(double value) {
        this.getIUnitStep().setZ(value);
    }

    public double getJUnitStepX() {
        return this.getJUnitStep().getX();
    }

    public double getJUnitStepY() {
        return this.getJUnitStep().getY();
    }

    public double getJUnitStepZ() {
        return this.getJUnitStep().getZ();
    }

    public void setJUnitStepX(double value) {
        this.getJUnitStep().setX(value);
    }

    public void setJUnitStepY(double value) {
        this.getJUnitStep().setY(value);
    }

    public void setJUnitStepZ(double value) {
        this.getJUnitStep().setZ(value);
    }

    public double getKUnitStepX() {
        return this.getKUnitStep().getX();
    }

    public double getKUnitStepY() {
        return this.getKUnitStep().getY();
    }

    public double getKUnitStepZ() {
        return this.getKUnitStep().getZ();
    }

    public void setKUnitStepX(double value) {
        this.getKUnitStep().setX(value);
    }

    public void setKUnitStepY(double value) {
        this.getKUnitStep().setY(value);
    }

    public void setKUnitStepZ(double value) {
        this.getKUnitStep().setZ(value);
    }

    /**
     * Create a new IJKGridDefinition.
     */

    public IJKGridDefinition() {
        this.Origin = new DoubleVector3();
        this.IUnitStep = new DoubleVector3();
        this.JUnitStep = new DoubleVector3();
        this.KUnitStep = new DoubleVector3();
    }

    /**
     * Create a copy of a IJKGridDefinition.
     *  
     */

    public IJKGridDefinition(IJKGridDefinition rhs) {
        this.Origin = new DoubleVector3(rhs.Origin);
        this.IUnitStep = new DoubleVector3(rhs.IUnitStep);
        this.JUnitStep = new DoubleVector3(rhs.JUnitStep);
        this.KUnitStep = new DoubleVector3(rhs.KUnitStep);
    }

    /**
     * Create a new IJKGridDefinition.
     * 
     * @param origin The value of 'Origin' for the new IJKGridDefinition
     * @param iunitStep The value of 'IUnitStep' for the new IJKGridDefinition
     * @param junitStep The value of 'JUnitStep' for the new IJKGridDefinition
     * @param kunitStep The value of 'KUnitStep' for the new IJKGridDefinition
     */

    public IJKGridDefinition(DoubleVector3 origin, DoubleVector3 iunitStep, DoubleVector3 junitStep, DoubleVector3 kunitStep) {
        this.Origin = new DoubleVector3(origin);
        this.IUnitStep = new DoubleVector3(iunitStep);
        this.JUnitStep = new DoubleVector3(junitStep);
        this.KUnitStep = new DoubleVector3(kunitStep);
    }
}
