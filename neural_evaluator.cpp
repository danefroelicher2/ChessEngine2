#include "neural_evaluator.h"
#include <iostream>
#include <cmath>
#include <cctype>

NeuralEvaluator::NeuralEvaluator() 
    : env(ORT_LOGGING_LEVEL_WARNING, "ChessEngine"),
      memoryInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)),
      initialized(false) {
}

NeuralEvaluator::~NeuralEvaluator() {
}

bool NeuralEvaluator::initialize(const std::string& modelPath) {
    try {
        // Create session options
        Ort::SessionOptions sessionOptions;
        sessionOptions.SetIntraOpNumThreads(1);
        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
        
        // Load the ONNX model
        session = std::make_unique<Ort::Session>(env, modelPath.c_str(), sessionOptions);
        
        // Set input/output names (must match export_model.py)
        inputNames = {"board_input"};
        outputNames = {"evaluation"};
        
        initialized = true;
        std::cout << "✓ Neural network loaded from: " << modelPath << std::endl;
        return true;
        
    } catch (const Ort::Exception& e) {
        std::cerr << "✗ Failed to load neural network: " << e.what() << std::endl;
        initialized = false;
        return false;
    }
}

std::vector<float> NeuralEvaluator::boardToFeatures(Board* board) {
    // Create 768-dimensional feature vector (12 piece types × 64 squares)
    // Order: WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK
    std::vector<float> features(768, 0.0f);
    
    // Map piece characters to feature indices
    auto getPieceIndex = [](char piece) -> int {
        switch(piece) {
            case 'P': return 0;   // White Pawn
            case 'N': return 1;   // White Knight
            case 'B': return 2;   // White Bishop
            case 'R': return 3;   // White Rook
            case 'Q': return 4;   // White Queen
            case 'K': return 5;   // White King
            case 'p': return 6;   // Black Pawn
            case 'n': return 7;   // Black Knight
            case 'b': return 8;   // Black Bishop
            case 'r': return 9;   // Black Rook
            case 'q': return 10;  // Black Queen
            case 'k': return 11;  // Black King
            default: return -1;
        }
    };
    
    // Iterate through board and set features
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = board->getPiece(row, col);
            if (piece == '.') continue;
            
            int pieceIndex = getPieceIndex(piece);
            if (pieceIndex < 0) continue;
            
            int squareIndex = row * 8 + col;
            int featureIndex = pieceIndex * 64 + squareIndex;
            
            features[featureIndex] = 1.0f;
        }
    }
    
    return features;
}

int NeuralEvaluator::evaluate(Board* board) {
    if (!initialized) {
        std::cerr << "✗ Neural evaluator not initialized!" << std::endl;
        return 0;
    }
    
    try {
        // Convert board to features
        std::vector<float> features = boardToFeatures(board);
        
        // Create input tensor
        std::vector<int64_t> inputShape = {1, 768};
        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo,
            features.data(),
            features.size(),
            inputShape.data(),
            inputShape.size()
        );
        
        // Run inference
        auto outputTensors = session->Run(
            Ort::RunOptions{nullptr},
            inputNames.data(),
            &inputTensor,
            1,
            outputNames.data(),
            1
        );
        
        // Extract output value (in pawns)
        float* outputData = outputTensors[0].GetTensorMutableData<float>();
        float evalInPawns = outputData[0];
        
        // Convert to centipawns (100 centipawns = 1 pawn)
        int evalInCentipawns = static_cast<int>(evalInPawns * 100.0f);
        
        return evalInCentipawns;
        
    } catch (const Ort::Exception& e) {
        std::cerr << "✗ Neural network inference failed: " << e.what() << std::endl;
        return 0;
    }
}
