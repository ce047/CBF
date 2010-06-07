/*
    This file is part of CBF.

    CBF is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    CBF is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CBF.  If not, see <http://www.gnu.org/licenses/>.


    Copyright 2009, 2010 Florian Paul Schmidt
*/

#include <cbf/kdl_transforms.h>

#include <cbf/utilities.h>
#include <cbf/debug_macros.h>
#include <cbf/plugin_macros.h>
#include <cbf/utilities.h>

#include <kdl/chain.hpp>
#include <kdl/tree.hpp>
#include <kdl/jntarray.hpp>
#include <kdl/frames.hpp>
#include <kdl/chainjnttojacsolver.hpp>
#include <kdl/chainfksolverpos_recursive.hpp>
#include <kdl/treejnttojacsolver.hpp>
#include <kdl/treefksolverpos_recursive.hpp>
#include <kdl/jntarrayvel.hpp>

namespace CBF {
	
	
	BaseKDLChainSensorTransform::BaseKDLChainSensorTransform(boost::shared_ptr<KDL::Chain> chain) :
		m_Chain(chain),
		m_Frame(new KDL::Frame),
		m_Jacobian(new KDL::Jacobian)
	{
		init_solvers();
	}
	
	void BaseKDLChainSensorTransform::init_solvers() {
	
		m_Twists = FloatMatrix(6, m_Chain->getNrOfJoints());
	
		m_JacSolver = boost::shared_ptr<KDL::ChainJntToJacSolver>(
			new KDL::ChainJntToJacSolver(*m_Chain)
		);
	
		m_FKSolver = boost::shared_ptr<KDL::ChainFkSolverPos_recursive>(
			new KDL::ChainFkSolverPos_recursive(*m_Chain)
		);
	
		m_Jacobian = boost::shared_ptr<KDL::Jacobian>(new KDL::Jacobian(m_Chain->getNrOfJoints()));
	}
	
	void BaseKDLChainSensorTransform::update() {
		//SensorTransform::update();
	
		KDL::JntArray jnt_array(resource_dim());
	
		//const ublas::vector<Float> &resource = m_Resource->get();
	
		CBF_DEBUG(m_Resource->get())
	
		for (unsigned int i = 0; i < resource_dim(); ++i) {
			jnt_array(i) = m_Resource->get()[i];
		}
	
		m_JacSolver->JntToJac(jnt_array, *m_Jacobian);
		m_FKSolver->JntToCart(jnt_array, *m_Frame);
	}
	
	unsigned int BaseKDLChainSensorTransform::resource_dim() const {
		return m_Chain->getNrOfJoints();
	}
	
	
	
	KDLChainPositionSensorTransform::KDLChainPositionSensorTransform(boost::shared_ptr<KDL::Chain> chain) :
		BaseKDLChainSensorTransform(chain)
	{
		m_Result = ublas::vector<Float>(3);
	}
	
	void KDLChainPositionSensorTransform::update() {
		BaseKDLChainSensorTransform::update();
	
		//! Update the jacobian so we can hand it out...
		m_TaskJacobian = FloatMatrix(3, resource_dim());
		for (unsigned int i = 0; i < 3; ++i) {
			for (unsigned int j = 0; j < resource_dim(); ++j) {
				m_TaskJacobian(i,j) = (*m_Jacobian)(i,j);
			}
		}
	
		//! Buffer result, so we can return it when requested...
		for (unsigned int i = 0; i < 3; ++i) {
			m_Result[i] = m_Frame->p(i);
		}
	}
	
	
	KDLChainAxisAngleSensorTransform::KDLChainAxisAngleSensorTransform(boost::shared_ptr<KDL::Chain> chain) :
		BaseKDLChainSensorTransform(chain)
	{
		m_Result = ublas::vector<Float>(3);
	}
	
	void KDLChainAxisAngleSensorTransform::update() {
		BaseKDLChainSensorTransform::update();
	
		CBF_DEBUG("Updating Jacobian")
		//! Update the jacobian so we can hand it out...
		m_TaskJacobian = ublas::zero_matrix<Float>(3, resource_dim());
	
		//CBF_DEBUG(m_ConcreteJacobian)
		//CBF_DEBUG(get_resource_dim())
	
		for (unsigned int i = 0; i < 3; ++i) {
			for (unsigned int j = 0; j < resource_dim(); ++j) {
				m_TaskJacobian(i,j) = (*m_Jacobian)(i+3, j);
				//CBF_DEBUG("la! " << m_ConcreteJacobian(i,j))
			}
		}
	
		CBF_DEBUG("m_ConcreteJacobian: " << m_TaskJacobian)
	
		//! Buffer result, so we can return it when requested...
		KDL::Vector vec;
		float angle;
		angle = (m_Frame->M).GetRotAngle(vec, 0.0000000000001);
	
		// angle = fmod(angle + 2.0 * M_PI, 2.0 * M_PI);
		if (angle > M_PI) angle -= 2.0 * M_PI;
		if (angle <= -M_PI) angle += 2.0 * M_PI;
	
		for (unsigned int i = 0; i < 3; ++i) {
			m_Result[i] = vec(i) * angle;
		}
	}
	




	BaseKDLTreeSensorTransform::BaseKDLTreeSensorTransform(
		boost::shared_ptr<KDL::Tree> tree,
		const std::vector<std::string> &segment_names
	) :
		m_Tree(tree),
		m_SegmentNames(segment_names)
	{
		init_solvers();

	}
	
	void BaseKDLTreeSensorTransform::init_solvers() {
	
		m_Twists = FloatMatrix(6, m_Tree->getNrOfJoints());
	
		m_JacSolver = boost::shared_ptr<KDL::TreeJntToJacSolver>(
			new KDL::TreeJntToJacSolver(*m_Tree)
		);
	
		m_FKSolver = boost::shared_ptr<KDL::TreeFkSolverPos_recursive>(
			new KDL::TreeFkSolverPos_recursive(*m_Tree)
		);

		for (unsigned int i = 0; i < m_SegmentNames.size(); ++i) {
			m_Jacobians.push_back(boost::shared_ptr<KDL::Jacobian>(new KDL::Jacobian(m_Tree->getNrOfJoints())));
			m_Frames.push_back(boost::shared_ptr<KDL::Frame>(new KDL::Frame));
		}
	}
	
	void BaseKDLTreeSensorTransform::update() {
		//SensorTransform::update();
	
		KDL::JntArray jnt_array(resource_dim());
	
		//const ublas::vector<Float> &resource = m_Resource->get();
	
		CBF_DEBUG(m_Resource->get())
	
		for (unsigned int i = 0; i < resource_dim(); ++i) {
			jnt_array(i) = m_Resource->get()[i];
		}

		for (unsigned int i = 0; i < m_SegmentNames.size(); ++i) {
			m_JacSolver->JntToJac(jnt_array, *m_Jacobians[i], m_SegmentNames[i]);
			m_FKSolver->JntToCart(jnt_array, *m_Frames[i], m_SegmentNames[i]);
		}
	
	}
	
	unsigned int BaseKDLTreeSensorTransform::resource_dim() const {
		return m_Tree->getNrOfJoints();
	}
	






	
	#ifdef CBF_HAVE_XSD
		BaseKDLChainSensorTransform::BaseKDLChainSensorTransform(const ChainBaseType &xml_instance, const SensorTransformType &xml_st_instance) :
			SensorTransform(xml_st_instance),
			m_Frame(new KDL::Frame),
			m_Jacobian(new KDL::Jacobian)
		{
			CBF_DEBUG("[KDLChainSensorTransform(const KDLChainSensorTransformType &xml_instance)]: yay!")
			m_Chain = create_chain(xml_instance);		
			init_solvers();
		}
		
		KDLChainPositionSensorTransform::KDLChainPositionSensorTransform(const KDLChainPositionSensorTransformType &xml_instance) :
			BaseKDLChainSensorTransform(xml_instance.Chain(), xml_instance)
		{
			//! TODO: recheck this function to make sure it works in all cases..
		
			m_Result = FloatVector(3);
		}
		
		KDLChainAxisAngleSensorTransform::KDLChainAxisAngleSensorTransform(
			const KDLChainAxisAngleSensorTransformType &xml_instance
		) :
			BaseKDLChainSensorTransform(xml_instance.Chain(), xml_instance)
		{
			//! TODO: recheck this function to make sure it works in all cases..
		
			m_Result = FloatVector(3);
		}


		BaseKDLTreeSensorTransform::BaseKDLTreeSensorTransform(const TreeBaseType &xml_instance, const SensorTransformType &xml_st_instance) :
			SensorTransform(xml_st_instance)
		{
			CBF_DEBUG("[KDLTreeSensorTransform(const KDLTreeSensorTransformType &xml_instance)]: yay!")

			m_Tree = create_tree(xml_instance);		
			init_solvers();
		}
		
		KDLTreePositionSensorTransform::KDLTreePositionSensorTransform(const KDLTreePositionSensorTransformType &xml_instance) :
			BaseKDLTreeSensorTransform(xml_instance.Tree(), xml_instance)
		{

		}
		
		KDLTreeAxisAngleSensorTransform::KDLTreeAxisAngleSensorTransform(
			const KDLTreeAxisAngleSensorTransformType &xml_instance
		) :
			BaseKDLTreeSensorTransform(xml_instance.Tree(), xml_instance)
		{

		}
	#endif



	
	CBF_PLUGIN_CLASS(KDLChainPositionSensorTransform, SensorTransform)
	CBF_PLUGIN_CLASS(KDLChainAxisAngleSensorTransform, SensorTransform)
} // namespace

