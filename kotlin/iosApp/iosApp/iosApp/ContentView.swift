import SwiftUI
import shared

struct ContentView: View {
    @State private var serverAddress: String = "192.168.1.18"
    @State private var serverPort: String = "5555"
    @State private var playerName: String = ""
    @State private var gameState: GameState = GameState()
    @State private var currentAnswer: String = ""
    @State private var errorMessage: String = ""
    @State private var isConnecting: Bool = false
    @State private var gameClient: GameClient?
    
    var body: some View {
        ZStack {
            Color(UIColor.systemBackground)
                .ignoresSafeArea()
            
            VStack(spacing: 16) {
                Text("NET-O-GRAMA")
                    .font(.system(size: 32, weight: .bold))
                    .foregroundColor(.blue)
                    .padding(.top, 16)
                
                if gameClient == nil {
                    // Connection setup screen
                    connectionSetupView()
                } else if gameState.currentPhase == GamePhase.waiting {
                    // Waiting for game to start
                    waitingView()
                } else if gameState.currentPhase == GamePhase.countdown {
                    // Countdown phase
                    countdownView()
                } else if gameState.currentPhase == GamePhase.playing {
                    // Game screen
                    gameView()
                } else if gameState.currentPhase == GamePhase.finished {
                    // Game finished
                    finishView()
                }
                
                if !errorMessage.isEmpty {
                    Text(errorMessage)
                        .foregroundColor(.red)
                        .padding()
                        .multilineTextAlignment(.center)
                }
                
                Spacer()
            }
            .padding()
        }
    }
    
    @ViewBuilder
    private func connectionSetupView() -> some View {
        VStack(spacing: 12) {
            TextField("Server Address", text: $serverAddress)
                .textFieldStyle(.roundedBorder)
                .padding(.horizontal)
            
            TextField("Server Port", text: $serverPort)
                .textFieldStyle(.roundedBorder)
                .padding(.horizontal)
                .keyboardType(.numberPad)
            
            TextField("Player Name", text: $playerName)
                .textFieldStyle(.roundedBorder)
                .padding(.horizontal)
            
            Button(action: connect) {
                Text(isConnecting ? "Connecting..." : "Connect")
                    .frame(maxWidth: .infinity)
                    .padding()
                    .background(Color.blue)
                    .foregroundColor(.white)
                    .cornerRadius(8)
            }
            .disabled(playerName.isEmpty || isConnecting || serverAddress.isEmpty)
            .padding(.horizontal)
            
            Spacer()
        }
        .padding()
    }
    
    @ViewBuilder
    private func waitingView() -> some View {
        VStack(spacing: 16) {
            Text("✓ Connected to Server")
                .font(.system(size: 16))
                .foregroundColor(.green)
            
            Text("Waiting for game to start...")
                .font(.system(size: 18))
            
            Button(action: { gameClient?.markReady() }) {
                Text("Ready to Play")
                    .frame(maxWidth: .infinity)
                    .padding()
                    .background(Color.blue)
                    .foregroundColor(.white)
                    .cornerRadius(8)
            }
            .padding(.horizontal)
            
            Button(action: disconnect) {
                Text("Disconnect")
                    .frame(maxWidth: .infinity)
                    .padding()
                    .background(Color.gray)
                    .foregroundColor(.white)
                    .cornerRadius(8)
            }
            .padding(.horizontal)
            
            Spacer()
        }
    }
    
    @ViewBuilder
    private func countdownView() -> some View {
        VStack(spacing: 16) {
            Text("Game Starting...")
                .font(.system(size: 28, weight: .bold))
                .foregroundColor(.blue)
            
            if !gameState.countdownMessage.isEmpty {
                Text(gameState.countdownMessage)
                    .font(.system(size: 72, weight: .bold))
                    .foregroundColor(countdownMessageColor())
            }
            
            Spacer()
        }
    }
    
    private func countdownMessageColor() -> Color {
        switch gameState.countdownMessage {
        case "READY":
            return .yellow
        case "STEADY":
            return .magenta
        case "GO!":
            return .green
        default:
            return .blue
        }
    }
    
    @ViewBuilder
    private func gameView() -> some View {
        VStack(spacing: 12) {
            // Shuffled word display
            Text(gameState.shuffledWord)
                .font(.system(size: 32, weight: .semibold))
                .tracking(4)
                .padding()
            
            // Timer
            Text("Time: \(gameState.timeRemaining)s")
                .font(.system(size: 18))
                .padding()
            
            // Players and scores
            if !gameState.players.isEmpty {
                HStack(spacing: 12) {
                    ForEach(gameState.players, id: \.name) { player in
                        VStack {
                            Text(player.name)
                                .font(.system(size: 12))
                            Text("\(player.score)")
                                .font(.system(size: 18, weight: .semibold))
                                .foregroundColor(.blue)
                        }
                        .frame(maxWidth: .infinity)
                        .padding(8)
                        .border(Color.gray)
                    }
                }
                .padding(.horizontal)
            }
            
            // Answer input and send button
            HStack(spacing: 8) {
                TextField("Your Answer", text: $currentAnswer)
                    .textFieldStyle(.roundedBorder)
                
                Button(action: { submitAnswer() }) {
                    Text("Send")
                        .frame(minWidth: 50)
                        .padding(.vertical, 8)
                        .padding(.horizontal, 12)
                        .background(Color.blue)
                        .foregroundColor(.white)
                        .cornerRadius(4)
                }
            }
            .padding(.horizontal)
            
            // Shuffle button
            Button(action: { gameClient?.shuffle() }) {
                Text("Shuffle")
                    .frame(maxWidth: .infinity)
                    .padding()
                    .background(Color.orange)
                    .foregroundColor(.white)
                    .cornerRadius(8)
            }
            .padding(.horizontal)
            
            // Word slots display
            if !gameState.wordSlots.isEmpty {
                VStack(alignment: .leading, spacing: 8) {
                    Text("Word Slots (\(gameState.wordSlots.filter { $0.playerIndex == -1 }.count) remaining):")
                        .font(.system(size: 14, weight: .semibold))
                        .padding(.leading)
                    
                    ScrollView {
                        VStack(spacing: 4) {
                            ForEach(0..<(gameState.wordSlots.count / 3 + 1), id: \.self) { rowIdx in
                                HStack(spacing: 0) {
                                    ForEach(0..<3, id: \.self) { colIdx in
                                        let slotIdx = rowIdx * 3 + colIdx
                                        if slotIdx < gameState.wordSlots.count {
                                            let slot = gameState.wordSlots[slotIdx]
                                            let slotColor = slotColorFor(playerIndex: slot.playerIndex)
                                            Text(slot.word)
                                                .font(.system(size: 12, design: .monospaced))
                                                .foregroundColor(slotColor)
                                                .frame(maxWidth: .infinity)
                                                .padding(4)
                                        } else {
                                            Spacer()
                                                .frame(maxWidth: .infinity)
                                                .padding(4)
                                        }
                                    }
                                }
                            }
                        }
                    }
                    .frame(maxHeight: 150)
                    .padding(8)
                    .border(Color.gray)
                }
                .padding(.horizontal)
            }
            
            // Quit button
            Button(action: disconnect) {
                Text("Quit Game")
                    .frame(maxWidth: .infinity)
                    .padding()
                    .background(Color.red)
                    .foregroundColor(.white)
                    .cornerRadius(8)
            }
            .padding(.horizontal)
            
            Spacer()
        }
    }
    
    private func slotColorFor(playerIndex: Int32) -> Color {
        switch playerIndex {
        case 0:
            return .red
        case 1:
            return .green
        case 2:
            return .yellow
        case 3:
            return .cyan
        default:
            return .gray
        }
    }
    
    @ViewBuilder
    private func finishView() -> some View {
        VStack(spacing: 16) {
            Text("Game Over!")
                .font(.system(size: 28, weight: .bold))
            
            let finalScores = gameState.players.sorted { $0.score > $1.score }
            if !finalScores.isEmpty {
                VStack(alignment: .leading, spacing: 8) {
                    ForEach(finalScores, id: \.name) { player in
                        HStack {
                            Text(player.name)
                            Spacer()
                            Text("\(player.score)")
                                .fontWeight(.bold)
                        }
                        .padding()
                    }
                }
                .border(Color.gray)
                .padding()
            }
            
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
    
    private func connect() {
        isConnecting = true
        let port = Int(serverPort) ?? 5555
        let client = GameClient(serverHost: serverAddress, serverPort: Int32(port))
        gameClient = client
        
        client.setGameStateListener { newState in
            DispatchQueue.main.async {
                gameState = newState
            }
        }
        
        client.setErrorListener { error in
            DispatchQueue.main.async {
                errorMessage = error
                isConnecting = false
            }
        }
        
        Task {
            do {
                try await client.connect(playerName: playerName)
                DispatchQueue.main.async {
                    isConnecting = false
                }
            } catch {
                DispatchQueue.main.async {
                    errorMessage = "Failed to connect: \(error.localizedDescription)"
                    gameClient = nil
                    isConnecting = false
                }
            }
        }
    }
    
    private func submitAnswer() {
        gameClient?.submitWord(word: currentAnswer)
        currentAnswer = ""
    }
    
    private func disconnect() {
        gameClient?.disconnect()
        gameClient = nil
        gameState = GameState()
        currentAnswer = ""
        errorMessage = ""
        isConnecting = false
    }
}

@available(iOS 14, *)
#Preview {
    ContentView()
}
