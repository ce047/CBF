/* -*- mode: c-non-suck; -*- */

#ifndef CONTROL_BASIS_POTENTIAL_HH
#define CONTROL_BASIS_POTENTIAL_HH

#include <cbf/types.h>
#include <cbf/utilities.h>

#include <boost/shared_ptr.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>

#include <vector>
#include <cassert>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <iostream>

#include <cbf/plugin_decl_macros.h>

CBF_PLUGIN_PREAMBLE(SquarePotential)
CBF_PLUGIN_PREAMBLE(CompositePotential)
CBF_PLUGIN_PREAMBLE(DirectionPotential)

namespace CBF {

namespace ublas = boost::numeric::ublas;

/**
	@brief This class represents an artificial potential function.

	The most important part of it is the gradient function. The 
	exec function ist just there for convenience in case the
	absolute value of the potential function needs to be known.
*/
struct Potential {
	/**
		DISTANCE_THRESHOLD means that the potential should signal
		convergence when only a certain distance away from at
		least one reference..

		Some potentials might not have a notion of reference. For 
		these convergence might be determined by a sufficiently
		small gradient step norm. For this the STEP_THRESHOLD 
		is useful.
	*/
	enum ConvergenceCriterion { DISTANCE_THRESHOLD = 1, STEP_THRESHOLD = 2 };
	unsigned int m_ConvergenceCriterion;

	Float m_DistanceThreshold;
	Float m_StepNormThreshold;

	Potential(
		int convergence_criterion = DISTANCE_THRESHOLD,
		Float distance_threshold = 0.01,
		Float stepnorm_threshold = 0.001
	) :
		m_ConvergenceCriterion(convergence_criterion),
		m_DistanceThreshold(distance_threshold),
		m_StepNormThreshold(stepnorm_threshold)
	{

	}

	virtual ~Potential() {

	}

	virtual Float norm(const FloatVector &v) = 0;

	virtual Float distance(const FloatVector &v1, const FloatVector &v2) = 0;

	/**
		@brief: Check for convergence of the potential

		The potential has to determine distance and stepnorm itself, as the
		norm in use might be very different from the euclidian norm (e.g. 
		for rotational spaces)
	*/ 
	virtual bool check_convergence(Float dist, Float stepnorm);

	/**
		The gradient can be implemented analytically or 
		numerically depending on taste and application

		Calling the gradient has to have the side effect of 
		determining whether the potential has converged
		or not..
	*/
	virtual void gradient (
		FloatVector &result, 
		const std::vector<FloatVector > &references, 
		const FloatVector &input
	) = 0;

	virtual bool converged() = 0;

	virtual unsigned int task_dim() = 0;
};

typedef boost::shared_ptr<Potential> PotentialPtr;


/**
	@brief The composite potential can be used to combine individual potentials
	into a single one.

	This is useful e.g. when the task space is a product
	of two rather different spaces (position and orientation for example).
*/
struct CompositePotential : public Potential {
	CBF_PLUGIN_DECL_METHODS(CompositePotential)

	/**
		@brief The potentials which are combined into a bigger one..
	*/
	std::vector<PotentialPtr> m_Potentials;

	//! @brief Buffers which are instance variables for efficiency reasons
	std::vector<FloatVector > m_in_buffers;

	//! @brief Buffers which are instance variables for efficiency reasons
	std::vector<FloatVector > m_out_buffers;

	//! @brief Buffers which are instance variables for efficiency reasons
	std::vector<FloatVector > m_ref_buffers;

	unsigned int m_Dim;

	CompositePotential(std::vector<PotentialPtr> potentials = std::vector<PotentialPtr>()) {
		set_potentials(potentials);
	}

	/**
		@brief Set the potentials to combine..
	*/
	void set_potentials(std::vector<PotentialPtr> &potentials) 
	{
		m_Potentials = potentials;
		m_in_buffers.resize(potentials.size());
		m_out_buffers.resize(potentials.size());
		m_ref_buffers.resize(potentials.size());
		m_Dim = 0;

		for (unsigned int i = 0; i < m_Potentials.size(); ++i) {
			m_Dim += m_Potentials[i]->task_dim();
			m_in_buffers[i] = ublas::zero_vector<Float>(m_Potentials[i]->task_dim());
			m_out_buffers[i] = ublas::zero_vector<Float>(m_Potentials[i]->task_dim());
			m_ref_buffers[i] = ublas::zero_vector<Float>(m_Potentials[i]->task_dim());
		}
	}

	virtual void gradient (
		FloatVector &result, 
		const std::vector<FloatVector > &references, 
		const FloatVector &input
	);

	/**
		We don't know how to combine the norms of the different potentials
		so we return 0..
	*/
	virtual Float norm(const FloatVector &v) {
		throw std::runtime_error("[CompositePotential]: Don't know how to calculate norm");
		return 0;
	}

	virtual Float distance(const FloatVector &v1, const FloatVector &v2) {
		throw std::runtime_error("[CompositePotential]: Don't know how to calculate distance");
		return 0;
	}

	/** 
		This implementation test if all of the potentials are converged and only then
		returns true
	*/
	virtual bool converged() {
		for (unsigned int i = 0; i < m_Potentials.size(); ++i)
			if (m_Potentials[i]->converged() == false)
				return false;

		return true;
	}

	virtual unsigned int task_dim() {
		return m_Dim;
	}

};

typedef boost::shared_ptr<CompositePotential> CompositePotentialPtr;




/**
	@brief A squared potential functions
*/
struct SquarePotential : public Potential {
	CBF_PLUGIN_DECL_METHODS(SquarePotential)

	Float m_Coefficient;
	Float m_MaxGradientStep;
	unsigned int m_Dim;

	bool m_Converged;

	SquarePotential(unsigned int dim = 1, Float coefficient = 1.0) :
		m_Coefficient(coefficient),
		m_MaxGradientStep(1.0),
		m_Dim(dim),
		m_Converged(false)
	{

	}

	virtual bool converged() {
		return m_Converged;		
	}

	virtual Float norm(const FloatVector &v) {
		return ublas::norm_2(v);
	}

	virtual Float distance(const FloatVector &v1, const FloatVector &v2) {
		// CBF_DEBUG("distance: " << (norm(v1 - v2)))
		return (norm(v1 - v2));
	}

	virtual unsigned int task_dim() {
		return m_Dim;
	}

	virtual void gradient (
		FloatVector &result,
		const std::vector<FloatVector > &references,
		const FloatVector &input
	) {
		//! First we find the closest reference vector
		Float min_dist = std::numeric_limits<Float>::max();
		unsigned int min_index = 0;

		//std::cout  << "[SquarePotential]: sizes: " << references[0].size() << " " << input.size() << std::endl;

		for (unsigned int i = 0; i < references.size(); ++i) {
			Float dist = distance(input, references[i]);
			if (dist < min_dist) {
				min_index = i;
				min_dist = dist;
			}
		}

		// CBF_DEBUG("min_index " << min_index)

		//! The gradient of a square function is just negative of
		//! input - reference..
		result = m_Coefficient * (references[min_index] - input);
		Float result_norm = norm(result);
		// CBF_DEBUG("result_norm " << result_norm)

		//! Normalize gradient step so it's not bigger than m_MaxGradientStep
		if (result_norm >= m_MaxGradientStep)
			result = (m_MaxGradientStep/result_norm) * result;
		// std::cout << "[SquaredPotential]: result: " << result << std::endl;

		m_Converged = check_convergence(distance(input, references[min_index]), norm(result));
		// CBF_DEBUG("m_Converged " << m_Converged)
	}
};

typedef boost::shared_ptr<SquarePotential> SquarePotentialPtr;


struct DirectionPotential {
	CBF_PLUGIN_DECL_METHODS(DirectionPotential)
};

} // namespace

#endif
