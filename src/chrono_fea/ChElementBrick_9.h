// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2014 projectchrono.org
// All right reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
// =============================================================================
// Authors: Radu Serban
// =============================================================================
// Brick element with 9 nodes (central node for curvature)
// =============================================================================

#ifndef CHELEMENTBRICK9_H
#define CHELEMENTBRICK9_H

#include "chrono/physics/ChContinuumMaterial.h"
#include "chrono/physics/ChSystem.h"
#include "chrono_fea/ChApiFEA.h"
#include "chrono_fea/ChElementGeneric.h"
#include "chrono_fea/ChNodeFEAcurv.h"
#include "chrono_fea/ChNodeFEAxyz.h"

namespace chrono {
namespace fea {

/// @addtogroup fea_elements
/// @{

/// Brick element with 9 nodes.
class ChApiFea ChElementBrick_9 : public ChElementGeneric, public ChLoadableUVW {
  public:
    ChElementBrick_9();
    ~ChElementBrick_9() {}

    /// Get number of nodes of this element.
    virtual int GetNnodes() override { return 9; }

    /// Get number of degrees of freedom of this element.
    virtual int GetNdofs() override { return 8 * 3 + 9; }

    /// Get the number of coordinates from the n-th node used by this element.
    virtual int GetNodeNdofs(int n) override {
        if (n < 8)
            return 3;

        return 9;
    }

    /// Access the n-th node of this element.
    virtual std::shared_ptr<ChNodeFEAbase> GetNodeN(int n) override {
        if (n < 8)
            return m_nodes[n];

        return m_central_node;
    }

    /// Specify the nodes of this element.
    void SetNodes(std::shared_ptr<ChNodeFEAxyz> node1,
                  std::shared_ptr<ChNodeFEAxyz> node2,
                  std::shared_ptr<ChNodeFEAxyz> node3,
                  std::shared_ptr<ChNodeFEAxyz> node4,
                  std::shared_ptr<ChNodeFEAxyz> node5,
                  std::shared_ptr<ChNodeFEAxyz> node6,
                  std::shared_ptr<ChNodeFEAxyz> node7,
                  std::shared_ptr<ChNodeFEAxyz> node8,
                  std::shared_ptr<ChNodeFEAcurv> nodeC);

    /// Get access to individual nodes of this element.
    std::shared_ptr<ChNodeFEAxyz> GetNode1() const { return m_nodes[0]; }
    std::shared_ptr<ChNodeFEAxyz> GetNode2() const { return m_nodes[1]; }
    std::shared_ptr<ChNodeFEAxyz> GetNode3() const { return m_nodes[2]; }
    std::shared_ptr<ChNodeFEAxyz> GetNode4() const { return m_nodes[3]; }
    std::shared_ptr<ChNodeFEAxyz> GetNode5() const { return m_nodes[4]; }
    std::shared_ptr<ChNodeFEAxyz> GetNode6() const { return m_nodes[5]; }
    std::shared_ptr<ChNodeFEAxyz> GetNode7() const { return m_nodes[6]; }
    std::shared_ptr<ChNodeFEAxyz> GetNode8() const { return m_nodes[7]; }
    std::shared_ptr<ChNodeFEAcurv> GetCentralNode() const { return m_central_node; }

    /// Set element dimensions (x, y, z directions).
    void SetDimensions(const ChVector<>& dims) { m_dimensions = dims; }
    /// Get the element dimensions (x, y, z directions).
    const ChVector<>& GetDimensions() const { return m_dimensions; }

    /// Set the continuum material for this element.
    void SetMaterial(std::shared_ptr<ChContinuumElastic> material) { m_material = material; }
    /// Get a handle to the continuum material used by this element.
    std::shared_ptr<ChContinuumElastic> GetMaterial() const { return m_material; }

    /// Enable/disable internal gravity calculation.
    void SetGravityOn(bool val) { m_gravity_on = val; }
    /// Check if internal gravity calculation is enabled/disabled.
    bool IsGravityOn() const { return m_gravity_on; }
	/// Set the structural damping.
	void SetAlphaDamp(double a) { m_Alpha = a; }

    /// Calculate shape functions and their derivatives.
    ///   N = [N1, N2, N3, N4, ...]                               (1x11 row vector)
    ///   S = [N1*eye(3), N2*eye(3), N3*eye(3) ,N4*eye(3), ...]   (3x11 matrix)
    void ShapeFunctions(ChMatrix<>& N, double x, double y, double z);
    void ShapeFunctionsDerivativeX(ChMatrix<>& Nx, double x, double y, double z);
    void ShapeFunctionsDerivativeY(ChMatrix<>& Ny, double x, double y, double z);
    void ShapeFunctionsDerivativeZ(ChMatrix<>& Nz, double x, double y, double z);

    /// Number of coordinates in the interpolated field: here the {x,y,z} displacement.
    virtual int Get_field_ncoords() override { return 3; }

    /// Tell the number of DOFs blocks: here 9, 1 for each node.
    virtual int GetSubBlocks() override { return 9; }

    /// Get the offset of the i-th sub-block of DOFs in global vector.
    virtual unsigned int GetSubBlockOffset(int nblock) override {
        if (nblock < 8)
            return m_nodes[nblock]->NodeGetOffset_w();
    
        return m_central_node->NodeGetOffset_w();
    }

    /// Get the size of the i-th sub-block of DOFs in global vector.
    virtual unsigned int GetSubBlockSize(int nblock) override {
        if (nblock < 8)
            return 3;

        return 9;
    }

    /// Get the number of DOFs affected by this element (position part).
    virtual int LoadableGet_ndof_x() override { return 8 * 3 + 9; }

    /// Get the number of DOFs affected by this element (speed part).
    virtual int LoadableGet_ndof_w() override { return 8 * 3 + 9; }

    /// Get all the DOFs packed in a single vector (position part).
    virtual void LoadableGetStateBlock_x(int block_offset, ChVectorDynamic<>& mD) override;

    /// Get all the DOFs packed in a single vector (speed part).
    virtual void LoadableGetStateBlock_w(int block_offset, ChVectorDynamic<>& mD) override;

    /// Get the pointers to the contained ChLcpVariables, appending to the mvars vector.
    virtual void LoadableGetVariables(std::vector<ChLcpVariables*>& mvars) override;

    /// Evaluate N'*F, where N is some type of shape function evaluated at (U,V,W).
    /// Here, U,V,W are coordinates of the volume, each ranging in -1..+1
    /// F is a load, N'*F is the resulting generalized load
    /// Returns also det[J] with J=[dx/du,..], that might be useful in gauss quadrature.
    virtual void ComputeNF(const double U,              ///< parametric coordinate in volume
                           const double V,              ///< parametric coordinate in volume
                           const double W,              ///< parametric coordinate in volume
                           ChVectorDynamic<>& Qi,       ///< Return result of N'*F  here, maybe with offset block_offset
                           double& detJ,                ///< Return det[J] here
                           const ChVectorDynamic<>& F,  ///< Input F vector, size is = n.field coords.
                           ChVectorDynamic<>* state_x,  ///< if != 0, update state (pos. part) to this, then evaluate Q
                           ChVectorDynamic<>* state_w   ///< if != 0, update state (speed part) to this, then evaluate Q
                           ) override;

    /// Return the material density.
    /// This is needed so that it can be accessed by ChLoaderVolumeGravity.
    virtual double GetDensity() override { return this->m_material->Get_density(); }

	virtual ChMatrixNM<double, 6, 6> GetE_eps() { return m_E_eps; }

  private:
    // -----------------------------------
    // Data
    // -----------------------------------

    std::vector<std::shared_ptr<ChNodeFEAxyz>> m_nodes;  ///< corner element nodes
    std::shared_ptr<ChNodeFEAcurv> m_central_node;       ///< central node

    std::shared_ptr<ChContinuumElastic> m_material;  ///< elastic naterial

    ChVector<> m_dimensions;                      ///< element dimensions (x, y, z components)
    bool m_gravity_on;                            ///< enable/disable internal gravity calculation
    ChMatrixNM<double, 33, 1> m_GravForce;        ///< gravitational force
    ChMatrixNM<double, 33, 33> m_MassMatrix;      ///< mass matrix
    ChMatrixNM<double, 33, 33> m_JacobianMatrix;  ///< Jacobian matrix (Kfactor*[K] + Rfactor*[R])
	double m_GaussScaling;
	double m_Alpha;                               ///< structural damping
    ChMatrixNM<double, 11, 3> m_d0;				  ///< initial nodal coordinates (in matrix form)
	ChMatrixNM<double, 11, 3> m_d;                ///< current nodal coordinates
	ChMatrixNM<double, 11, 11> m_ddT;             ///< matrix m_d * m_d^T
	ChMatrixNM<double, 11, 11> m_d0d0T;           ///< matrix m_d0 * m_d0^T
	ChMatrixNM<double, 33, 1> m_d_dt;             ///< current nodal velocities
	ChMatrixNM<double, 6, 6> m_E_eps;

    // -----------------------------------
    // Interface to base classes
    // -----------------------------------

    /// Update this element.
    virtual void Update() override;

    /// Fill the D vector (column matrix) with the current states of the element nodes.
    virtual void GetStateBlock(ChMatrixDynamic<>& mD);

    /// Initial element setup.
    virtual void SetupInitial(ChSystem* system) override;
    /// Set M as the global mass matrix.
    virtual void ComputeMmatrixGlobal(ChMatrix<>& M) override;
    /// Set H as the global stiffness matrix K, scaled  by Kfactor. Optionally, also
    /// superimposes global damping matrix R, scaled by Rfactor, and global mass matrix M multiplied by Mfactor.
    virtual void ComputeKRMmatricesGlobal(ChMatrix<>& H, double Kfactor, double Rfactor = 0, double Mfactor = 0) override;

    /// Compute internal forces and load them in the Fi vector.
    virtual void ComputeInternalForces(ChMatrixDynamic<>& Fi) override;

    // -----------------------------------
    // Functions for internal computations
    // -----------------------------------

    /// Compute the mass matrix of the element.
    void ComputeMassMatrix();
    /// Compute the gravitational forces.
    void ComputeGravityForce(const ChVector<>& g_acc);

    /// Compute Jacobians of the internal forces.
    /// This function calculates a linear combination of the stiffness (K) and damping (R) matrices,
    ///     J = Kfactor * K + Rfactor * R
    /// for given coeficients Kfactor and Rfactor.
    /// This Jacobian will be further combined with the global mass matrix M and included in the global
    /// stiffness matrix H in the function ComputeKRMmatricesGlobal().
    void ComputeInternalJacobians(double Kfactor, double Rfactor);

    /// Calculate the determinant of the initial configuration.
    double Calc_detJ0(double x, double y, double z);

    /// Calculate the determinant of the initial configuration.
    /// Same as above, but also return the dense shape function vector derivatives.
    double Calc_detJ0(double x,
                      double y,
                      double z,
                      ChMatrixNM<double, 1, 11>& Nx,
                      ChMatrixNM<double, 1, 11>& Ny,
                      ChMatrixNM<double, 1, 11>& Nz,
                      ChMatrixNM<double, 1, 3>& Nx_d0,
                      ChMatrixNM<double, 1, 3>& Ny_d0,
                      ChMatrixNM<double, 1, 3>& Nz_d0);

    // Calculate the current 11x3 matrix of nodal coordinates.
    void CalcCoordMatrix(ChMatrixNM<double, 11, 3>& d);

	// Calculate the current 33x1 matrix of nodal coordinate derivatives.
	void CalcCoordDerivMatrix(ChMatrixNM<double, 33, 1>& dt);

    friend class MyMassBrick9;
    friend class MyGravityBrick9;
    friend class MyForceBrick9;
    friend class MyJacobianBrick9;
};

/// @} fea_elements

}  // end namespace fea
}  // end namespace chrono

#endif
