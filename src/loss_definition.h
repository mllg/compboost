// ========================================================================== //
//                                 ___.                          __           //
//        ____  ____   _____ ______\_ |__   ____   ____  _______/  |_         //
//      _/ ___\/  _ \ /     \\____ \| __ \ /  _ \ /  _ \/  ___/\   __\        //
//      \  \__(  <_> )  Y Y  \  |_> > \_\ (  <_> |  <_> )___ \  |  |          //
//       \___  >____/|__|_|  /   __/|___  /\____/ \____/____  > |__|          //
//           \/            \/|__|       \/                  \/                //
//                                                                            //
// ========================================================================== //
//
// Compboost is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// Compboost is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with Compboost. If not, see <http://www.gnu.org/licenses/>.
//
// This file contains:
// -------------------
//
//   This file contains the different loss implementations. The structure here
//   is:
//     - Parent class 'LossDefinition' which are virtual member functions. This
//       functions are later overwritten by the child member functions which
//       contains the concrete implementation of the loss.
//
//     - Child classes have the structure:
//
//         class SpecificLoss: public LossDefinition
//         {
//           arma::vec DefinedLoss      { IMPLEMENTATION };
//           arma::vec DefinedGradient  { IMPLEMENTATION };
//           double ConstantInitializer { IMPLEMENTATION };
//         }
//
//     - There is one special child class, the 'CustomLoss' which allows to
//       define custom loss functions out of R.
//
//
// Written by:
// -----------
//
//   Daniel Schalk
//   Institut für Statistik
//   Ludwig-Maximilians-Universität München
//   Ludwigstraße 33
//   D-80539 München
//
//   https://www.compstat.statistik.uni-muenchen.de
//
// ========================================================================== //

#ifndef LOSSDEFINITION_H_
#define LOSSDEFINITION_H_

#include <RcppArmadillo.h>

#include <iostream>

namespace lossdef 
{

// Parent class:
// -----------------------

class LossDefinition
{
  public:

    virtual arma::vec DefinedLoss (arma::vec &true_value, arma::vec &prediction) = 0;
    virtual arma::vec DefinedGradient (arma::vec &true_value, arma::vec &prediction) = 0;
    virtual double ConstantInitializer (arma::vec &true_value) = 0;
};

// -------------------------------------------------------------------------- //
// Loss implementations as child classes:
// -------------------------------------------------------------------------- //

// Quadratic loss:
// -----------------------

class Quadratic: public LossDefinition
{
  public:

    arma::vec DefinedLoss (arma::vec &true_value, arma::vec &prediction)
    {
      // for debugging:
      std::cout << "Calculate loss of child class Quadratic!" << std::endl;
      return arma::pow(true_value - prediction, 2) / 2;
    }

    arma::vec DefinedGradient (arma::vec &true_value, arma::vec &prediction)
    {
      // for debugging:
      std::cout << "Calculate gradient of child class Quadratic!" << std::endl;
      return true_value - prediction;
    }

    double ConstantInitializer (arma::vec &true_value)
    {
      return arma::mean(true_value);
    }
};

// Absolute loss:
// -----------------------

class Absolute: public LossDefinition
{
  public:

    arma::vec DefinedLoss (arma::vec &true_value, arma::vec &prediction)
    {
      // for debugging:
      std::cout << "Calculate loss of child class Absolute!" << std::endl;
      return arma::abs(true_value - prediction);
    }

    arma::vec DefinedGradient (arma::vec &true_value, arma::vec &prediction)
    {
      // for debugging:
      std::cout << "Calculate gradient of child class Absolute!" << std::endl;
      return arma::sign(true_value - prediction);
    }

    double ConstantInitializer (arma::vec &true_value)
    {
      return arma::median(true_value);
    }
};

// Custom loss:
// -----------------------

// This one is a special one. It allows to use a custom loss predefined in R.
// The convenience here comes from the 'Rcpp::Function' class and the use of
// a special constructor which defines the three needed functions!

// Note that there is one conversion step. There is no predefined conversion
// from 'Rcpp::Function' (which acts as SEXP) to 'double'. But it is possible
// to go the step above 'Rcpp::NumericVector'. Therefore the custom functions
// returns a 'Rcpp::NumericVector' which then is able to be converted to
// 'double' by just selecting one element.

class CustomLoss: public LossDefinition
{
  private:

    Rcpp::Function lossFun;
    Rcpp::Function gradientFun;
    Rcpp::Function initFun;

  public:

    CustomLoss (Rcpp::Function lossFun, Rcpp::Function gradientFun, Rcpp::Function initFun) :
      lossFun( lossFun ), gradientFun( gradientFun ), initFun( initFun )
    {
      std::cout << "Be careful! You are using a custom loss out of R!"
                << "This will slow down everything!"
                << std::endl;
    }

    arma::vec DefinedLoss (arma::vec &true_value, arma::vec &prediction)
    {
      // for debugging:
      std::cout << "Calculate loss for a custom loss!" << std::endl;
      Rcpp::NumericVector out = lossFun(true_value, prediction);
      return out;
    }

    arma::vec DefinedGradient (arma::vec &true_value, arma::vec &prediction)
    {
      // for debugging:
      std::cout << "Calculate gradient for a custom loss!" << std::endl;
      Rcpp::NumericVector out = gradientFun(true_value, prediction);
      return out;
    }

    // Conversion step from 'SEXP' to double via 'Rcpp::NumericVector' which 
    // knows how to convert a 'SEXP':
    double ConstantInitializer (arma::vec &true_value)
    {
      Rcpp::NumericVector out = initFun(true_value);
      return out[1];
    }
};

} // namespace lossdef

#endif // LOSSDEFINITION_H_