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
// it under the terms of the MIT License.
// Compboost is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// MIT License for more details. You should have received a copy of 
// the MIT License along with compboost. 
//
// Written by:
// -----------
//
//   Daniel Schalk
//   Department of Statistics
//   Ludwig-Maximilians-University Munich
//   Ludwigstrasse 33
//   D-80539 München
//
//   https://www.compstat.statistik.uni-muenchen.de
//
//   Contact
//   e: contact@danielschalk.com
//   w: danielschalk.com
//
// =========================================================================== #

#include "compboost.h"

namespace cboost {

// --------------------------------------------------------------------------- #
// Constructor:
// --------------------------------------------------------------------------- #

// todo: response as call by reference!

Compboost::Compboost () {}

Compboost::Compboost (const arma::vec& response, const double& learning_rate, 
  const bool& stop_if_all_stopper_fulfilled, optimizer::Optimizer* used_optimizer, 
  loss::Loss* used_loss, loggerlist::LoggerList* used_logger0,
  blearnerlist::BaselearnerFactoryList used_baselearner_list)
  : response ( response ), 
    learning_rate ( learning_rate ),
    stop_if_all_stopper_fulfilled ( stop_if_all_stopper_fulfilled ),
    used_optimizer ( used_optimizer ),
    used_loss ( used_loss ),
    used_baselearner_list ( used_baselearner_list )
{
  blearner_track = blearnertrack::BaselearnerTrack(learning_rate);
  used_logger["initial.training"] = used_logger0;
}

// --------------------------------------------------------------------------- #
// Member functions:
// --------------------------------------------------------------------------- #

void Compboost::train (const unsigned int& trace, const arma::vec& prediction, loggerlist::LoggerList* logger)
{

  if (used_baselearner_list.getMap().size() == 0) {
    Rcpp::stop("Could not train without any registered base-learner.");
  }
  
  arma::vec pred_temp = prediction;
  arma::vec blearner_pred_temp;
  
  bool stop_the_algorithm = false;
  unsigned int k = 1;
  
  // Main Algorithm. While the stop criteria isn't fulfilled, run the 
  // algorithm:
  while (! stop_the_algorithm) {

    actual_iteration = blearner_track.getBaselearnerVector().size() + 1;
    
    // Define pseudo residuals as negative gradient:
    pseudo_residuals = -used_loss->definedGradient(response, pred_temp);
    
    // Cast integer k to string for baselearner identifier:
    std::string temp_string = std::to_string(k);
    blearner::Baselearner* selected_blearner = used_optimizer->findBestBaselearner(temp_string, pseudo_residuals, used_baselearner_list.getMap());

    // Prediction is needed more often, use a temp vector to avoid multiple computations:
    blearner_pred_temp = selected_blearner->predict();

    used_optimizer->calculateStepSize(used_loss, response, pred_temp, blearner_pred_temp);
    
    // Insert new baselearner to vector of selected baselearner. The parameter are estimated here, hence
    // the contribution to the old parameter is the estimated parameter times the learning rate times
    // the step size. Therefore we have to pass the step size which changes in each iteration:    
    blearner_track.insertBaselearner(selected_blearner, used_optimizer->getStepSize(actual_iteration));

    // Update model (prediction) and shrink by learning rate:
    pred_temp += learning_rate * used_optimizer->getStepSize(actual_iteration) * blearner_pred_temp;
    
    // Log the current step:    
    //   The last term has to be the prediction or anything like that. This is
    //   important to track the risk (inbag or oob)!!!!    
    logger->logCurrent(k, response, pred_temp, selected_blearner, initialization, learning_rate, used_optimizer->getStepSize(actual_iteration));
    
    // Calculate and log risk:
    risk.push_back(arma::mean(used_loss->definedLoss(response, pred_temp)));

    // Get status of the algorithm (is the stopping criteria reached?). The negation here
    // seems a bit weird, but it makes the while loop easier to read:
    stop_the_algorithm = ! logger->getStopperStatus(stop_if_all_stopper_fulfilled);
    
    if (trace > 0) {
      if ((k == 1) || ((k % trace) == 0)) {
        logger->printLoggerStatus(risk.back()); 
      }
    }    
    k += 1;
  }

  if (trace) {
    Rcpp::Rcout << std::endl;
    Rcpp::Rcout << std::endl; 
  }

  model_prediction = pred_temp;  
}

void Compboost::trainCompboost (const unsigned int& trace)
{
  // Make sure, that the selected baselearner and logger data is empty:
  blearner_track.clearBaselearnerVector();
  for (auto& it : used_logger) {
    it.second->clearLoggerData();
  }
  
  // Initialize zero model and pseudo residuals:
  initialization = used_loss->constantInitializer(response);
  arma::vec pseudo_residuals_init (response.size());
  // Rcpp::Rcout << "<<Compboost>> Initialize zero model and pseudo residuals" << std::endl;
  
  // Initialize prediction and fill with zero model:
  arma::vec prediction(response.size());
  prediction.fill(initialization);
  // Rcpp::Rcout << "<<Compboost>> Initialize prediction and fill with zero model" << std::endl;
  
  // Calculate risk for initial model:
  risk.push_back(arma::mean(used_loss->definedLoss(response, prediction)));

  // track time:
  auto t1 = std::chrono::high_resolution_clock::now();
  
  // Initial training:
  train(trace, prediction, used_logger["initial.training"]);
  
  // track time:
  auto t2 = std::chrono::high_resolution_clock::now();
  
  // After training call printer for a status:
  Rcpp::Rcout << "Train " << std::to_string(actual_iteration) << " iterations in " 
              << std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count() 
              << " Seconds." << std::endl;
  Rcpp::Rcout << "Final risk based on the train set: " << std::setprecision(2) 
              << risk.back() << std::endl << std::endl;
  
  // Set flag if model is trained:
  model_is_trained = true;
}

void Compboost::continueTraining (loggerlist::LoggerList* logger, const unsigned int& trace)
{
  if (! model_is_trained) {
    Rcpp::stop("Initial training hasn't been done yet. Use 'train()' first.");
  }
  // Set state to maximal possible iteration to cleanly continue training:
  if (actual_iteration != blearner_track.getBaselearnerVector().size()) {
    
    unsigned int max_iteration = blearner_track.getBaselearnerVector().size();

    // Rcpp::Rcout << "Set iteration to maximal possible value: " << std::to_string(max_iteration) << std::endl;
    
    setToIteration(max_iteration);
    
  }
  
  // Continue training:
  train(trace, model_prediction, logger);
  
  // Register logger in hash map to store logging data:
  std::string logger_id = "retraining" + std::to_string(used_logger.size());
  used_logger[logger_id] = logger;
  
  // Update actual state:
  actual_iteration = blearner_track.getBaselearnerVector().size();
}

arma::vec Compboost::getPrediction (const bool& as_response) const
{
  arma::vec pred;
  if (as_response) {
    pred = used_loss->responseTransformation(model_prediction);
  } else {
    pred = model_prediction;
  }
  return pred;
}

std::map<std::string, arma::mat> Compboost::getParameter () const
{
  return blearner_track.getParameterMap();
}

std::vector<std::string> Compboost::getSelectedBaselearner () const
{
  std::vector<std::string> selected_blearner;
  
  for (unsigned int i = 0; i < actual_iteration; i++) {
    selected_blearner.push_back(blearner_track.getBaselearnerVector()[i]->getDataIdentifier() + "_" + blearner_track.getBaselearnerVector()[i]->getBaselearnerType());
  }
  return selected_blearner;
}

std::map<std::string, loggerlist::LoggerList*> Compboost::getLoggerList () const
{
  return used_logger;
}

std::map<std::string, arma::mat> Compboost::getParameterOfIteration (const unsigned int& k) const 
{
  // Check is done in function GetEstimatedParameterOfIteration in baselearner_track.cpp 
  return blearner_track.getEstimatedParameterOfIteration(k);
}

std::pair<std::vector<std::string>, arma::mat> Compboost::getParameterMatrix () const
{
  return blearner_track.getParameterMatrix();
}

arma::vec Compboost::predict () const
{
  std::map<std::string, arma::mat> parameter_map  = blearner_track.getParameterMap();
  // std::map<std::string, arma::mat> train_data_map = used_baselearner_list.getDataMap();
  
  arma::vec pred(model_prediction.n_elem);
  pred.fill(initialization);
  
  // Calculate vector - matrix product for each selected base-learner:
  for (auto& it : parameter_map) {    
    std::string sel_factory = it.first;
    pred += used_baselearner_list.getMap().find(sel_factory)->second->getData() * it.second;
    // pred += train_data_map.find(sel_factory)->second * it.second;    
  }
  return pred;
}

// Predict for new data. Note: The data_map contains the raw columns of the used data.
// Those columns are then transformed by the corresponding transform data function of the
// specific factory. After the transformation, the transformed data is multiplied by the
// corresponding parameter.
arma::vec Compboost::predict (std::map<std::string, data::Data*> data_map, const bool& as_response) const
{
  // IMPROVE THIS FUNCTION!!! See: 
  // https://github.com/schalkdaniel/compboost/issues/206
  
  std::map<std::string, arma::mat> parameter_map = blearner_track.getParameterMap();

  arma::vec pred(data_map.begin()->second->getData().n_rows);
  pred.fill(initialization);
  
  // Rcpp::Rcout << "initialize pred vec" << std::endl;
  
  // Idea is simply to calculate the vector matrix product of parameter and 
  // newdata. The problem here is that the newdata comes as raw data and has
  // to be transformed first:
  for (auto& it : parameter_map) {
    
    // Name of current feature:
    std::string sel_factory = it.first;
 
    // Find the element with key 'hat'
    blearnerfactory::BaselearnerFactory* sel_factory_obj = used_baselearner_list.getMap().find(sel_factory)->second;
      
    // Select newdata corresponding to selected facotry object:
    std::map<std::string, data::Data*>::iterator it_newdata;
    it_newdata = data_map.find(sel_factory_obj->getDataIdentifier());

    // Calculate prediction by accumulating the design matrices multiplied by the estimated parameter:
    if (it_newdata != data_map.end()) {
      arma::mat data_trafo = sel_factory_obj->instantiateData(it_newdata->second->getData());
      pred += data_trafo * it.second;
    }
  }
  if (as_response) {
    pred = used_loss->responseTransformation(pred);
  }
  return pred;
}

arma::vec Compboost::predictionOfIteration (std::map<std::string, data::Data*> data_map, const unsigned int& k, const bool& as_response) const
{
  // Rcpp::Rcout << "Get into Compboost::predict" << std::endl;
  
  // Check is done in function GetEstimatedParameterOfIteration in baselearner_track.cpp 
  std::map<std::string, arma::mat> parameter_map = blearner_track.getEstimatedParameterOfIteration(k);
  
  arma::vec pred(data_map.begin()->second->getData().n_rows);
  pred.fill(initialization);
  
  // Rcpp::Rcout << "initialize pred vec" << std::endl;
  
  for (auto& it : parameter_map) {
    
    std::string sel_factory = it.first;
    
    // Rcpp::Rcout << "Fatory id of parameter map: " << sel_factory << std::endl;
    
    blearnerfactory::BaselearnerFactory* sel_factory_obj = used_baselearner_list.getMap().find(sel_factory)->second;
    
    // Rcpp::Rcout << "Data of selected factory: " << sel_factory_obj->GetDataIdentifier() << std::endl;
    
    arma::mat data_trafo = sel_factory_obj->instantiateData((data_map.find(sel_factory_obj->getDataIdentifier())->second->getData()));
    pred += data_trafo * it.second;
    
  }
  if (as_response) {
    pred = used_loss->responseTransformation(pred);
  }
  return pred;
}

// Set model to an given iteration. The predictions and everything is then done at this iteration:
void Compboost::setToIteration (const unsigned int& k) 
{
  unsigned int max_iteration = blearner_track.getBaselearnerVector().size();
  
  // Set parameter:
  if (k > max_iteration) {
    // Define new iteration logger for missing iterations:
    unsigned int iteration_diff = k - max_iteration;  
    logger::Logger* temp_logger = new logger::LoggerIteration("_iteration", true, iteration_diff);
    loggerlist::LoggerList* temp_loggerlist = new loggerlist::LoggerList();
    
    // Register that logger:
    std::string logger_id = "setToIteration.retraining" + std::to_string(used_logger.size());
    temp_loggerlist->registerLogger(temp_logger);
    
    Rcpp::Rcout << "\nYou have already trained " << std::to_string(max_iteration) << " iterations.\n" 
                <<"Train " << std::to_string(iteration_diff) << " additional iterations."
                << std::endl << std::endl;
    
    continueTraining(temp_loggerlist, false);
  } 
  
  blearner_track.setToIteration(k);
  
  // Set prediction:
  model_prediction = predict();
  
  // Set actual state:
  actual_iteration = k;
}

double Compboost::getOffset() const 
{
  return initialization;
}

std::vector<double> Compboost::getRiskVector () const
{
  return risk;
}

void Compboost::summarizeCompboost () const
{
  Rcpp::Rcout << "Compboost object with:" << std::endl;
  Rcpp::Rcout << "\t- Learning Rate: " << learning_rate << std::endl;
  Rcpp::Rcout << "\t- Are all logger used as stopper: " << stop_if_all_stopper_fulfilled << std::endl;
  
  if (model_is_trained) {
    Rcpp::Rcout << "\t- Model is already trained with " << blearner_track.getBaselearnerVector().size() << " iterations/fitted baselearner" << std::endl;
    Rcpp::Rcout << "\t- Actual state is at iteration " << actual_iteration << std::endl;
    Rcpp::Rcout << "\t- Loss optimal initialization: " << std::fixed << std::setprecision(2) << initialization << std::endl;
  }
  Rcpp::Rcout << std::endl;
  Rcpp::Rcout << "To get more information check the other objects!" << std::endl;
}

// Destructor:
Compboost::~Compboost ()
{
  // blearner_track will be deleted automatically (allocated on the stack)
  
  // used_logger will be deleted automatically (allocated on the stack). BUT we
  // have to care about self registered logger by setToIteration:
  for (auto& it : used_logger) {
    if (it.first.find("setToIteration") != std::string::npos) {
      // Delets the loggerlist:
      delete it.second;
    }
  }
}

} // namespace cboost
