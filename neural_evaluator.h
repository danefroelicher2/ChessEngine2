#ifndef NEURAL_EVALUATOR_H
#define NEURAL_EVALUATOR_H

#include <vector>
#include <string>
#include <memory>
#include "board.h"
#include <onnxruntime_cxx_api.h>

class NeuralEvaluator {
public:
    NeuralEvaluator();
    ~NeuralEvaluator();
    
    // Initialize the ONNX model
    bool initialize(const std::string& modelPath);
    
    // Evaluate a board position using the neural network
    // Returns evaluation in centipawns (e.g., +100 = 1 pawn advantage for white)
    int evaluate(Board* board);
    
    // Check if model is loaded
    bool isInitialized() const { return initialized; }
    
private:
    // Convert board to 768-dimensional feature vector
    std::vector<float> boardToFeatures(Board* board);
    
    // ONNX Runtime components
    Ort::Env env;
    std::unique_ptr<Ort::Session> session;
    Ort::MemoryInfo memoryInfo;
    
    bool initialized;
    
    // Input/output names for ONNX model
    std::vector<const char*> inputNames;
    std::vector<const char*> outputNames;
};

#endif // NEURAL_EVALUATOR_H
