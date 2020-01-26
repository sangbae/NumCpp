/// @file
/// @author David Pilger <dpilger26@gmail.com>
/// [GitHub Repository](https://github.com/dpilger26/NumCpp)
/// @version 1.2
///
/// @section License
/// Copyright 2019 David Pilger
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this
/// software and associated documentation files(the "Software"), to deal in the Software
/// without restriction, including without limitation the rights to use, copy, modify,
/// merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
/// permit persons to whom the Software is furnished to do so, subject to the following
/// conditions :
///
/// The above copyright notice and this permission notice shall be included in all copies
/// or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
/// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
/// PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
/// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
/// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/// DEALINGS IN THE SOFTWARE.
///
/// @section Description
/// Functions for working with NdArrays
///
#pragma once

#include "NumCpp/Core/Constants.hpp"
#include "NumCpp/Core/Types.hpp"
#include "NumCpp/Utils/sqr.hpp"

#include <cmath>
#include <functional>
#include <vector>

namespace nc
{
    class LegendrePolynomial 
    {
    public:
        LegendrePolynomial(const double lowerBound, const double upperBound, const uint32 numberOfIterations) :
            lowerBound_(lowerBound), 
            upperBound_(upperBound),
            numIterations_(numberOfIterations),
            weight_(numberOfIterations + 1),
            root_(numberOfIterations + 1)
        {
            calculateWeightAndRoot();
        }

        const std::vector<double>& getWeight() const 
        {
            return weight_;
        }

        const std::vector<double>& getRoot() const
        {
            return root_;
        }

    private:
        const double EPSILON{1e-15};

        struct Result 
        {
            double value{0.0};
            double derivative{0.0};

            Result(const double val, const double deriv) :
                value(val), 
                derivative(deriv) 
            {}
        };

        void calculateWeightAndRoot()
        {
            double numIterationsDouble = static_cast<double>(numIterations_);
            for(uint32 step = 0; step <= numIterations_; ++step) 
            {
                double root = std::cos(constants::pi * (static_cast<double>(step) - 0.25) / (numIterationsDouble + 0.5));
                Result result = calculatePolynomialValueAndDerivative(root);

                double newtonRaphsonRatio;
                do 
                {
                    newtonRaphsonRatio = result.value / result.derivative;
                    root -= newtonRaphsonRatio;
                    result = calculatePolynomialValueAndDerivative(root);
                } 
                while (std::fabs(newtonRaphsonRatio) > EPSILON);

                root_[step] = root;
                weight_[step] = 2.0 / ((1.0 - utils::sqr(root)) * result.derivative * result.derivative);
            }
        }

        Result calculatePolynomialValueAndDerivative(const double x)
        {
            Result result(x, 0.0);

            double value_minus_1 = 1.0;
            const double f = 1.0 / (utils::sqr(x) - 1.0);
            for(uint32 step = 2; step <= numIterations_; ++step)
            {
                const double stepDouble = static_cast<double>(step);
                const double value = ((2.0 * stepDouble - 1.0) * x * result.value - (stepDouble - 1.0) * value_minus_1) / stepDouble;
                result.derivative = stepDouble * f * (x * value - result.value);

                value_minus_1 = result.value;
                result.value = value;
            }

            return result;
        }

        const double        lowerBound_;
        const double        upperBound_;
        const uint32        numIterations_;
        std::vector<double> weight_;
        std::vector<double> root_;
    };

    //============================================================================
    // Method Description:
    ///						Performs Gauss-Legendre integration of the input function
    ///
    /// @param				low: the lower bound of the integration
    /// @param              high: the upper bound of the integration
    /// @param				numIterations: the number of iterations to perform
    /// @param              function: the function to integrate over
    ///
    /// @return             double
    ///
    inline double integrate_gauss_legendre(const double low, const double high, const uint32 numIterations,
        const std::function<double (double)>& f)
    {
        const LegendrePolynomial legendrePolynomial(low, high, numIterations);
        const std::vector<double>& weight = legendrePolynomial.getWeight();
        const std::vector<double>& root = legendrePolynomial.getRoot();

        const double width = 0.5 * (high - low);
        const double mean = 0.5 * (low + high);

        double gaussLegendre = 0.0;
        for(uint32 step = 1; step <= numIterations; ++step)
        {
            gaussLegendre += weight[step] * f(width * root[step] + mean);
        }

        return gaussLegendre * width;
    }
}