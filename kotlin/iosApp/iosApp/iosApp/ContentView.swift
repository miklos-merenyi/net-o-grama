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
            LinearGradient(
                gradient: Gradient(colors: [
                    Color(UIColor.systemBackground),
                    Color(UIColor.systemBackground).opacity(0.95)
                ]),
                startPoint: .topLeading,
                endPoint: .bottomTrailing
            )
            .ignoresSafeArea()
            
            VStack(spacing: 0) {
                if gameClient == nil {
                    connectionSetupView()
                } else if gameState.currentPhase == GamePhase.waiting {
                    waitingView()
                } else if gameState.currentPhase == GamePhase.countdown {
                    countdownView()
                } else if gameState.currentPhase == GamePhase.playing {
                    gameView()
                } else if gameState.currentPhase == GamePhase.finished {
                    finishView()
                }
                
                Spacer()
            }
            
            if !errorMessage.isEmpty {
                VStack {
                    Spacer()
                    VStack(spacing: 8) {
                        HStack {
                            Image(systemName: "exclamationmark.circle.fill")
                                .foregroundColor(.red)
                            Text(errorMessage)
                                .lineLimit(3)
                        }
                        .font(.system(size: 14))
                    }
                    .frame(maxWidth: .infinity)
                    .padding(16)
                    .background(Color(UIColor.systemRed).opacity(0.1))
                    .border(Color.red, width: 1)
                    .cornerRadius(12)
                    .padding(16)
                }
            }
        }
    }
    
    @ViewBuilder
    private func connectionSetupView() -> some View {
        VStack(spacing: 0) {
            // Header
            VStack(spacing: 12) {
                Image(systemName: "play.circle.fill")
                    .font(.system(size: 64))
                    .foregroundColor(.blue)
                
                Text("No-Grama")
                    .font(.system(size: 40, weight: .bold))
                    .foregroundColor(.blue)
                
                Text("Word Game Challenge")
                    .font(.system(size: 14))
                    .foregroundColor(.secondary)
            }
            .frame(maxWidth: .infinity)
            .padding(.vertical, 32)
            
            // Connection form
            VStack(spacing: 16) {
                VStack(alignment: .leading, spacing: 4) {
                    Label("Server Address", systemImage: "network")
                        .font(.system(size: 12, weight: .semibold))
                        .foregroundColor(.secondary)
                    
                    TextField("e.g., 192.168.1.100", text: $serverAddress)
                        .textFieldStyle(.roundedBorder)
                        .font(.system(size: 16))
                }
                
                VStack(alignment: .leading, spacing: 4) {
                    Label("Server Port", systemImage: "gear")
                        .font(.system(size: 12, weight: .semibold))
                        .foregroundColor(.secondary)
                    
                    TextField("e.g., 5555", text: $serverPort)
                        .textFieldStyle(.roundedBorder)
                        .keyboardType(.numberPad)
                        .font(.system(size: 16))
                }
                
                VStack(alignment: .leading, spacing: 4) {
                    Label("Player Name", systemImage: "person")
                        .font(.system(size: 12, weight: .semibold))
                        .foregroundColor(.secondary)
                    
                    TextField("Enter your name", text: $playerName)
                        .textFieldStyle(.roundedBorder)
                        .font(.system(size: 16))
                }
                
                Button(action: connect) {
                    HStack(spacing: 8) {
                        if isConnecting {
                            ProgressView()
                                .tint(.white)
                        } else {
                            Image(systemName: "link")
                        }
                        Text(isConnecting ? "Connecting..." : "Join Game")
                            .font(.system(size: 16, weight: .semibold))
                    }
                    .frame(maxWidth: .infinity)
                    .padding(16)
                    .background(Color.blue)
                    .foregroundColor(.white)
                    .cornerRadius(12)
                }
                .disabled(playerName.isEmpty || isConnecting || serverAddress.isEmpty)
            }
            .padding(24)
            .background(Color(UIColor.secondarySystemBackground))
            .cornerRadius(16)
            .padding(20)
            
            Button(action: { exit(0) }) {
                HStack(spacing: 8) {
                    Image(systemName: "xmark.circle.fill")
                    Text("Exit")
                        .font(.system(size: 16, weight: .semibold))
                }
                .frame(maxWidth: .infinity)
                .padding(14)
                .background(Color(UIColor.systemRed).opacity(0.1))
                .foregroundColor(.red)
                .cornerRadius(12)
                .overlay(
                    RoundedRectangle(cornerRadius: 12)
                        .stroke(Color.red.opacity(0.3), lineWidth: 1)
                )
            }
            .padding(.horizontal, 20)
            .padding(.top, 8)
        }
    }
    
    @ViewBuilder
    private func waitingView() -> some View {
        VStack(spacing: 24) {
            VStack(spacing: 16) {
                VStack(spacing: 12) {
                    Image(systemName: "checkmark.circle.fill")
                        .font(.system(size: 56))
                        .foregroundColor(.green)
                    
                    Text("Connected!")
                        .font(.system(size: 28, weight: .bold))
                    
                    Text("Waiting for other players...")
                        .font(.system(size: 16))
                        .foregroundColor(.secondary)
                }
                .frame(maxWidth: .infinity)
                .padding(32)
                .background(Color(UIColor.secondarySystemBackground))
                .cornerRadius(16)
            }
            .padding(20)
            
            VStack(spacing: 12) {
                Button(action: { gameClient?.markReady() }) {
                    HStack(spacing: 8) {
                        Image(systemName: "hand.thumbsup.fill")
                        Text("Ready to Play")
                            .font(.system(size: 16, weight: .semibold))
                    }
                    .frame(maxWidth: .infinity)
                    .padding(16)
                    .background(Color.blue)
                    .foregroundColor(.white)
                    .cornerRadius(12)
                }
                
                Button(action: disconnect) {
                    HStack(spacing: 8) {
                        Image(systemName: "power")
                        Text("Disconnect")
                            .font(.system(size: 16, weight: .semibold))
                    }
                    .frame(maxWidth: .infinity)
                    .padding(14)
                    .background(Color(UIColor.systemRed).opacity(0.1))
                    .foregroundColor(.red)
                    .cornerRadius(12)
                    .overlay(
                        RoundedRectangle(cornerRadius: 12)
                            .stroke(Color.red.opacity(0.3), lineWidth: 1)
                    )
                }
            }
            .padding(20)
            
            Spacer()
        }
    }
    
    @ViewBuilder
    private func countdownView() -> some View {
        VStack(spacing: 32) {
            Text("Game Starting")
                .font(.system(size: 32, weight: .semibold))
                .foregroundColor(.blue)
            
            if !gameState.countdownMessage.isEmpty {
                Text(gameState.countdownMessage)
                    .font(.system(size: 100, weight: .bold))
                    .foregroundColor(countdownMessageColor())
                    .transition(.scale.combined(with: .opacity))
            } else {
                Text("Get ready!")
                    .font(.system(size: 24))
                    .foregroundColor(.secondary)
            }
            
            Spacer()
        }
        .frame(maxWidth: .infinity)
        .padding(32)
    }
    
    private func countdownMessageColor() -> Color {
        switch gameState.countdownMessage {
        case "READY":
            return Color(red: 1.0, green: 0.84, blue: 0)
        case "STEADY":
            return Color(red: 1.0, green: 0.08, blue: 0.58)
        case "GO!":
            return Color(red: 0.2, green: 0.84, blue: 0.2)
        default:
            return .blue
        }
    }
    
    @ViewBuilder
    private func gameView() -> some View {
        ScrollView {
            VStack(spacing: 16) {
                // Current word card
                VStack(spacing: 12) {
                    HStack {
                        VStack(alignment: .leading, spacing: 4) {
                            Text("Current Word")
                                .font(.system(size: 12, weight: .semibold))
                                .foregroundColor(.secondary)
                            Text(gameState.shuffledWord)
                                .font(.system(size: 36, weight: .bold))
                                .foregroundColor(.blue)
                                .tracking(2)
                        }
                        
                        Spacer()
                        
                        VStack(alignment: .trailing, spacing: 4) {
                            Text("Time")
                                .font(.system(size: 12, weight: .semibold))
                                .foregroundColor(.white)
                            Text("\(gameState.timeRemaining)s")
                                .font(.system(size: 28, weight: .bold))
                                .foregroundColor(.white)
                        }
                        .padding(12)
                        .background(Color.blue)
                        .cornerRadius(10)
                    }
                }
                .padding(16)
                .background(Color(UIColor.secondarySystemBackground))
                .cornerRadius(14)
                .padding(.horizontal, 16)
                .padding(.vertical, 8)
                
                // Player scores
                if !gameState.players.isEmpty {
                    VStack(alignment: .leading, spacing: 8) {
                        Text("Scores")
                            .font(.system(size: 14, weight: .semibold))
                            .foregroundColor(.secondary)
                            .padding(.horizontal, 16)
                        
                        ScrollView(.horizontal, showsIndicators: false) {
                            HStack(spacing: 12) {
                                ForEach(Array(gameState.players.enumerated()), id: \.element.name) { idx, player in
                                    playerScoreCard(player: player, index: idx)
                                }
                            }
                            .padding(.horizontal, 16)
                        }
                    }
                    .padding(.vertical, 8)
                }
                
                // Input and buttons
                VStack(spacing: 12) {
                    HStack(spacing: 8) {
                        TextField("Your Answer", text: $currentAnswer)
                            .textFieldStyle(.roundedBorder)
                            .font(.system(size: 16))
                        
                        Button(action: { submitAnswer() }) {
                            Image(systemName: "paperplane.fill")
                                .font(.system(size: 16))
                                .frame(minWidth: 44)
                                .frame(height: 44)
                                .background(Color.blue)
                                .foregroundColor(.white)
                                .cornerRadius(10)
                        }
                    }
                    .padding(12)
                    
                    HStack(spacing: 12) {
                        Button(action: { gameClient?.shuffle() }) {
                            HStack(spacing: 6) {
                                Image(systemName: "arrow.triangle.2.circlepath")
                                Text("Shuffle")
                                    .font(.system(size: 14, weight: .semibold))
                            }
                            .frame(maxWidth: .infinity)
                            .padding(12)
                            .background(Color.orange)
                            .foregroundColor(.white)
                            .cornerRadius(10)
                        }
                        
                        Button(action: disconnect) {
                            HStack(spacing: 6) {
                                Image(systemName: "xmark.circle.fill")
                                Text("Quit")
                                    .font(.system(size: 14, weight: .semibold))
                            }
                            .frame(maxWidth: .infinity)
                            .padding(12)
                            .background(Color(UIColor.systemRed).opacity(0.1))
                            .foregroundColor(.red)
                            .cornerRadius(10)
                            .overlay(
                                RoundedRectangle(cornerRadius: 10)
                                    .stroke(Color.red.opacity(0.3), lineWidth: 1)
                            )
                        }
                    }
                    .padding(12)
                }
                .background(Color(UIColor.secondarySystemBackground))
                .cornerRadius(14)
                .padding(.horizontal, 16)
                
                // Word slots
                if !gameState.wordSlots.isEmpty {
                    VStack(alignment: .leading, spacing: 8) {
                        Text("Found Words (​\(gameState.wordSlots.filter { $0.playerIndex != -1 }.count)/\(gameState.wordSlots.count))")
                            .font(.system(size: 14, weight: .semibold))
                            .foregroundColor(.secondary)
                            .padding(.horizontal, 16)
                        
                        VStack(spacing: 8) {
                            ForEach(0..<(gameState.wordSlots.count / 3 + 1), id: \.self) { rowIdx in
                                HStack(spacing: 8) {
                                    ForEach(0..<3, id: \.self) { colIdx in
                                        let slotIdx = rowIdx * 3 + colIdx
                                        if slotIdx < gameState.wordSlots.count {
                                            let slot = gameState.wordSlots[slotIdx]
                                            wordSlotCard(slot: slot)
                                        } else {
                                            Spacer()
                                        }
                                    }
                                }
                            }
                        }
                        .padding(12)
                        .background(Color(UIColor.tertiarySystemBackground))
                        .cornerRadius(10)
                        .padding(.horizontal, 16)
                    }
                    .padding(.vertical, 8)
                }
            }
            .padding(.vertical, 8)
        }
    }
    
    private func playerScoreCard(player: Player, index: Int) -> some View {
        let playerColors: [Color] = [
            Color(red: 1.0, green: 0.42, blue: 0.42),
            Color(red: 0.32, green: 0.81, blue: 0.4),
            Color(red: 1.0, green: 0.85, blue: 0.25),
            Color(red: 0.42, green: 0.75, blue: 0.88)
        ]
        let color = playerColors[index % playerColors.count]
        
        return VStack(spacing: 8) {
            Circle()
                .fill(color)
                .frame(width: 24, height: 24)
            
            Text(player.name)
                .font(.system(size: 12, weight: .semibold))
                .lineLimit(1)
            
            Text("\(player.score)")
                .font(.system(size: 18, weight: .bold))
                .foregroundColor(color)
        }
        .frame(minWidth: 100)
        .padding(12)
        .background(Color(UIColor.secondarySystemBackground))
        .cornerRadius(12)
    }
    
    private func wordSlotCard(slot: WordSlot) -> some View {
        let playerColors: [Color] = [
            Color(red: 1.0, green: 0.42, blue: 0.42),
            Color(red: 0.32, green: 0.81, blue: 0.4),
            Color(red: 1.0, green: 0.85, blue: 0.25),
            Color(red: 0.42, green: 0.75, blue: 0.88)
        ]
        let slotColor = slot.playerIndex >= 0 && slot.playerIndex < 4 
            ? playerColors[Int(slot.playerIndex) % playerColors.count] 
            : Color.secondary
        
        return VStack {
            Text(slot.word)
                .font(.system(size: 11, weight: slot.playerIndex == -1 ? .regular : .bold, design: .monospaced))
                .foregroundColor(slot.playerIndex == -1 ? Color.secondary : slotColor)
                .lineLimit(1)
        }
        .frame(maxWidth: .infinity)
        .padding(8)
        .background(slot.playerIndex == -1 
            ? Color(UIColor.systemBackground) 
            : Color(UIColor.secondarySystemBackground))
        .cornerRadius(8)
    }
    
    @ViewBuilder
    private func finishView() -> some View {
        VStack(spacing: 24) {
            VStack(spacing: 12) {
                Image(systemName: "checkmark.circle.fill")
                    .font(.system(size: 56))
                    .foregroundColor(.green)
                
                Text("Game Over!")
                    .font(.system(size: 32, weight: .bold))
            }
            .padding(32)
            
            if !gameState.players.isEmpty {
                let finalScores = gameState.players.sorted { $0.score > $1.score }
                VStack(spacing: 12) {
                    Text("Final Scores")
                        .font(.system(size: 16, weight: .semibold))
                        .frame(maxWidth: .infinity, alignment: .leading)
                        .padding(.horizontal, 16)
                    
                    ForEach(Array(finalScores.enumerated()), id: \.element.name) { idx, player in
                        HStack {
                            Text("\(idx + 1)")
                                .font(.system(size: 18, weight: .bold))
                                .foregroundColor(.blue)
                                .frame(width: 32)
                            
                            Text(player.name)
                                .font(.system(size: 16, weight: .semibold))
                            
                            Spacer()
                            
                            Text("\(player.score)")
                                .font(.system(size: 18, weight: .bold))
                                .foregroundColor(.blue)
                        }
                        .padding(12)
                        .background(Color(UIColor.secondarySystemBackground))
                        .cornerRadius(10)
                    }
                }
                .padding(.horizontal, 16)
            }
            
            Button(action: disconnect) {
                HStack(spacing: 8) {
                    Image(systemName: "xmark.circle.fill")
                    Text("Close")
                        .font(.system(size: 16, weight: .semibold))
                }
                .frame(maxWidth: .infinity)
                .padding(16)
                .background(Color.red)
                .foregroundColor(.white)
                .cornerRadius(12)
            }
            .padding(20)
            
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
