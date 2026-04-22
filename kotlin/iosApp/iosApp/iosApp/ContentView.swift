import SwiftUI
import shared

struct ContentView: View {
    @State private var playerName: String = ""
    @State private var gameState: GameState = GameState()
    @State private var currentAnswer: String = ""
    @State private var errorMessage: String = ""
    @State private var isConnecting: Bool = false
    
    private let gameClient = GameClient(host: "localhost", port: Int32(12345))
    
    var body: some View {
        ZStack {
            Color(UIColor.systemBackground)
                .ignoresSafeArea()
            
            VStack(spacing: 16) {
                Text("NET-O-GRAMA")
                    .font(.system(size: 32, weight: .bold))
                    .foregroundColor(.blue)
                    .padding(.top, 32)
                
                if gameState.currentPhase == GamePhase.waiting {
                    connectionView()
                } else if gameState.currentPhase == GamePhase.playing {
                    gameView()
                } else if gameState.currentPhase == GamePhase.finished {
                    finishView()
                }
                
                if !errorMessage.isEmpty {
                    Text(errorMessage)
                        .foregroundColor(.red)
                        .padding(.horizontal)
                        .multilineTextAlignment(.center)
                }
                
                Spacer()
            }
            .padding()
        }
        .onAppear {
            setupGameClient()
        }
    }
    
    @ViewBuilder
    private func connectionView() -> some View {
        VStack(spacing: 16) {
            TextField("Player Name", text: $playerName)
                .textFieldStyle(.roundedBorder)
                .padding()
            
            Button(action: connect) {
                Text(isConnecting ? "Connecting..." : "Connect")
                    .frame(maxWidth: .infinity)
                    .padding()
                    .background(Color.blue)
                    .foregroundColor(.white)
                    .cornerRadius(8)
            }
            .disabled(playerName.isEmpty || isConnecting)
            .padding()
            
            Spacer()
        }
    }
    
    @ViewBuilder
    private func gameView() -> some View {
        VStack(spacing: 16) {
            Text("Root Word: \(gameState.rootWord)")
                .font(.system(size: 18))
                .padding()
            
            Text(gameState.shuffledWord)
                .font(.system(size: 36, weight: .bold))
                .tracking(4)
                .padding()
            
            Text("Time: \(gameState.timeRemaining)s")
                .font(.system(size: 18))
            
            TextField("Your Answer", text: $currentAnswer)
                .textFieldStyle(.roundedBorder)
                .padding()
            
            HStack(spacing: 12) {
                Button(action: submitAnswer) {
                    Text("Submit")
                        .frame(maxWidth: .infinity)
                        .padding()
                        .background(Color.green)
                        .foregroundColor(.white)
                        .cornerRadius(8)
                }
                
                Button(action: shuffle) {
                    Text("Shuffle")
                        .frame(maxWidth: .infinity)
                        .padding()
                        .background(Color.orange)
                        .foregroundColor(.white)
                        .cornerRadius(8)
                }
            }
            .padding()
            
            VStack(alignment: .leading, spacing: 8) {
                Text("Anagrams Found:")
                    .font(.headline)
                
                ScrollView {
                    VStack(alignment: .leading, spacing: 4) {
                        ForEach(gameState.anagrams, id: \.self) { anagram in
                            Text(anagram)
                                .font(.system(size: 14))
                                .padding(4)
                        }
                    }
                }
            }
            .padding()
            .border(Color.gray, width: 1)
            
            Spacer()
        }
    }
    
    @ViewBuilder
    private func finishView() -> some View {
        VStack(spacing: 16) {
            Text("Game Over!")
                .font(.system(size: 28, weight: .bold))
            
            Text("Score: \(gameState.score)")
                .font(.system(size: 20))
            
            Button(action: disconnect) {
                Text("Disconnect")
                    .frame(maxWidth: .infinity)
                    .padding()
                    .background(Color.red)
                    .foregroundColor(.white)
                    .cornerRadius(8)
            }
            .padding()
            
            Spacer()
        }
    }
    
    private func setupGameClient() {
        gameClient.setGameStateListener { newState in
            DispatchQueue.main.async {
                gameState = newState
            }
        }
        
        gameClient.setErrorListener { error in
            DispatchQueue.main.async {
                errorMessage = error
            }
        }
    }
    
    private func connect() {
        isConnecting = true
        // Connection logic would go here
        // For now, just update state for demo
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) {
            isConnecting = false
        }
    }
    
    private func submitAnswer() {
        gameClient.submitWord(word: currentAnswer)
        currentAnswer = ""
    }
    
    private func shuffle() {
        gameClient.shuffle()
    }
    
    private func disconnect() {
        gameClient.disconnect()
    }
}

@available(iOS 14, *)
struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
