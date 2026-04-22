package com.nograma

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Send
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.launch

class MainActivity : ComponentActivity() {
    private var gameClient: GameClient? = null
    private var soundManager: SoundManager? = null
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        // Initialize sound manager
        soundManager = SoundManager(this)
        
        setContent {
            GameScreen(this) { host, port, playerName ->
                val client = GameClient(host, port)
                gameClient = client
                // Set up sound callback
                client.setSoundListener { soundName ->
                    soundManager?.playSound(soundName)
                }
                client  // Return client without calling connect - it's called from button
            }
        }
    }
    
    override fun onDestroy() {
        super.onDestroy()
        gameClient?.disconnect()
        soundManager?.release()
    }
}

@Composable
fun GameScreen(activity: MainActivity, createGameClient: (String, Int, String) -> GameClient) {
    var gameClient by remember { mutableStateOf<GameClient?>(null) }
    var gameState by remember { mutableStateOf(GameState()) }
    var playerName by remember { mutableStateOf("") }
    var serverAddress by remember { mutableStateOf("192.168.1.18") }
    var serverPort by remember { mutableStateOf("5555") }
    var currentAnswer by remember { mutableStateOf("") }
    var isConnecting by remember { mutableStateOf(false) }
    var errorMessage by remember { mutableStateOf("") }
    val coroutineScope = rememberCoroutineScope()
    
    LaunchedEffect(gameClient) {
        gameClient?.let { client ->
            client.setGameStateListener { newState ->
                // Update game state when server sends new state
                gameState = newState
            }
            client.setErrorListener { error ->
                errorMessage = error
            }
        }
    }
    
    MaterialTheme {
        Surface(
            modifier = Modifier
                .fillMaxSize()
                .background(MaterialTheme.colorScheme.background)
        ) {
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(16.dp),
                horizontalAlignment = Alignment.CenterHorizontally,
                verticalArrangement = Arrangement.Center
            ) {
                if (gameClient == null) {
                    // Connection setup screen
                    TextField(
                        value = serverAddress,
                        onValueChange = { serverAddress = it },
                        label = { Text("Server Address") },
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(8.dp),
                        placeholder = { Text("e.g., 192.168.1.100 or localhost") }
                    )
                    
                    TextField(
                        value = serverPort,
                        onValueChange = { serverPort = it },
                        label = { Text("Server Port") },
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(8.dp),
                        placeholder = { Text("e.g., 5555") },
                        keyboardOptions = KeyboardOptions(imeAction = ImeAction.Done)
                    )
                    
                    TextField(
                        value = playerName,
                        onValueChange = { playerName = it },
                        label = { Text("Player Name") },
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(8.dp)
                    )
                    
                    Button(
                        onClick = {
                            isConnecting = true
                            val port = serverPort.toIntOrNull() ?: 5555
                            val client = createGameClient(serverAddress, port, playerName)
                            gameClient = client
                            
                            // Launch connection in coroutine
                            coroutineScope.launch {
                                try {
                                    client.connect(playerName)
                                    isConnecting = false
                                } catch (e: Exception) {
                                    errorMessage = "Failed to connect: ${e.message}"
                                    gameClient = null
                                    isConnecting = false
                                }
                            }
                        },
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(8.dp),
                        enabled = !isConnecting && serverAddress.isNotEmpty() && playerName.isNotEmpty()
                    ) {
                        Text(if (isConnecting) "Connecting..." else "Connect")
                    }
                    
                    Button(
                        onClick = { 
                            activity.finish()
                        },
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(8.dp),
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MaterialTheme.colorScheme.error
                        )
                    ) {
                        Text("Quit")
                    }
                } else if (gameState.currentPhase == GamePhase.WAITING) {
                    // Waiting for game to start - show connection status
                    Text(
                        "✓ Connected to Server",
                        fontSize = 16.sp,
                        color = MaterialTheme.colorScheme.primary,
                        modifier = Modifier.padding(16.dp)
                    )
                    
                    Text(
                        "Waiting for game to start...",
                        fontSize = 18.sp,
                        modifier = Modifier.padding(16.dp)
                    )
                    
                    Button(
                        onClick = { gameClient?.markReady() },
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(8.dp),
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MaterialTheme.colorScheme.primary
                        )
                    ) {
                        Text("Ready to Play")
                    }
                    
                    Button(
                        onClick = { 
                            gameClient?.disconnect()
                            gameClient = null
                            isConnecting = false
                            errorMessage = ""
                        },
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(8.dp)
                    ) {
                        Text("Disconnect")
                    }
                } else if (gameState.currentPhase == GamePhase.COUNTDOWN) {
                    // Countdown phase
                    Text(
                        "Game Starting...",
                        fontSize = 28.sp,
                        color = MaterialTheme.colorScheme.primary,
                        modifier = Modifier.padding(16.dp)
                    )
                    
                    // Display countdown message if available
                    if (gameState.countdownMessage.isNotEmpty()) {
                        Text(
                            gameState.countdownMessage,
                            fontSize = 72.sp,
                            fontWeight = androidx.compose.ui.text.font.FontWeight.Bold,
                            color = when (gameState.countdownMessage) {
                                "READY" -> androidx.compose.ui.graphics.Color.Yellow
                                "STEADY" -> androidx.compose.ui.graphics.Color.Magenta
                                "GO!" -> androidx.compose.ui.graphics.Color.Green
                                else -> MaterialTheme.colorScheme.primary
                            },
                            modifier = Modifier.padding(32.dp),
                            textAlign = androidx.compose.ui.text.style.TextAlign.Center
                        )
                    } else {
                        Text(
                            "Get ready!",
                            fontSize = 20.sp,
                            modifier = Modifier.padding(16.dp)
                        )
                    }
                } else if (gameState.currentPhase == GamePhase.PLAYING) {
                    // Game screen
                    Text(
                        gameState.shuffledWord,
                        fontSize = 32.sp,
                        modifier = Modifier.padding(16.dp)
                    )
                    
                    Text(
                        "Time: ${gameState.timeRemaining}s",
                        fontSize = 18.sp,
                        modifier = Modifier.padding(8.dp)
                    )
                    
                    // Players and scores
                    if (gameState.players.isNotEmpty()) {
                        Text("Players:", fontSize = 14.sp, modifier = Modifier.padding(top = 12.dp, start = 16.dp, bottom = 4.dp))
                        Row(
                            modifier = Modifier
                                .fillMaxWidth()
                                .padding(horizontal = 16.dp),
                            horizontalArrangement = Arrangement.spacedBy(16.dp)
                        ) {
                            gameState.players.forEach { player ->
                                Card(
                                    modifier = Modifier
                                        .weight(1f)
                                        .padding(4.dp)
                                ) {
                                    Column(
                                        modifier = Modifier
                                            .fillMaxWidth()
                                            .padding(8.dp),
                                        horizontalAlignment = Alignment.CenterHorizontally
                                    ) {
                                        Text(
                                            player.name,
                                            fontSize = 12.sp,
                                            modifier = Modifier.padding(bottom = 4.dp)
                                        )
                                        Text(
                                            "${player.score}",
                                            fontSize = 18.sp,
                                            color = MaterialTheme.colorScheme.primary,
                                            modifier = Modifier.padding(top = 4.dp)
                                        )
                                    }
                                }
                            }
                        }
                    }
                    
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(8.dp),
                        horizontalArrangement = Arrangement.spacedBy(8.dp)
                    ) {
                        TextField(
                            value = currentAnswer,
                            onValueChange = { currentAnswer = it },
                            label = { Text("Your Answer") },
                            modifier = Modifier
                                .weight(1f),
                            keyboardOptions = KeyboardOptions(imeAction = ImeAction.Send),
                            keyboardActions = KeyboardActions(
                                onSend = {
                                    gameClient?.submitWord(currentAnswer)
                                    currentAnswer = ""
                                }
                            )
                        )
                        
                        Button(
                            onClick = { 
                                gameClient?.submitWord(currentAnswer)
                                currentAnswer = ""
                            },
                            modifier = Modifier.align(Alignment.CenterVertically)
                        ) {
                            Icon(Icons.Filled.Send, contentDescription = "Send")
                        }
                    }
                    
                    Button(
                        onClick = { gameClient?.shuffle() },
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(8.dp)
                    ) {
                        Text("Shuffle")
                    }
                    
                    // Display word slots (places to guess) - like Python client's GUESS area
                    if (gameState.wordSlots.isNotEmpty()) {
                        Text("Word Slots (${gameState.wordSlots.count { it.playerIndex == -1 }} remaining):", fontSize = 16.sp, modifier = Modifier.padding(top = 8.dp, start = 16.dp, end = 16.dp))
                        LazyColumn(
                            modifier = Modifier
                                .fillMaxWidth()
                                .heightIn(max = 200.dp)
                                .padding(horizontal = 8.dp)
                        ) {
                            items(gameState.wordSlots.size / 3 + 1) { rowIdx ->
                                Row(
                                    modifier = Modifier
                                        .fillMaxWidth()
                                        .padding(vertical = 2.dp),
                                    horizontalArrangement = Arrangement.SpaceEvenly
                                ) {
                                    repeat(3) { colIdx ->
                                        val slotIdx = rowIdx * 3 + colIdx
                                        
                                        if (slotIdx < gameState.wordSlots.size) {
                                            val slot = gameState.wordSlots[slotIdx]
                                            val slotColor = if (slot.playerIndex == -1) {
                                                androidx.compose.ui.graphics.Color.LightGray
                                            } else {
                                                when (slot.playerIndex) {
                                                    0 -> androidx.compose.ui.graphics.Color.Red
                                                    1 -> androidx.compose.ui.graphics.Color.Green
                                                    2 -> androidx.compose.ui.graphics.Color.Yellow
                                                    3 -> androidx.compose.ui.graphics.Color.Cyan
                                                    else -> androidx.compose.ui.graphics.Color.LightGray
                                                }
                                            }
                                            Text(
                                                slot.word,
                                                fontSize = 12.sp,
                                                modifier = Modifier.padding(4.dp),
                                                color = slotColor,
                                                fontFamily = androidx.compose.ui.text.font.FontFamily.Monospace
                                            )
                                        } else {
                                            Spacer(modifier = Modifier.weight(1f))
                                        }
                                    }
                                }
                            }
                        }
                    }
                    
                    Button(
                        onClick = { 
                            gameClient?.disconnect()
                            gameClient = null
                            isConnecting = false
                            currentAnswer = ""
                            errorMessage = ""
                        },
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(8.dp),
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MaterialTheme.colorScheme.error
                        )
                    ) {
                        Text("Quit Game")
                    }
                }
                
                if (errorMessage.isNotEmpty()) {
                    Text(
                        errorMessage,
                        color = MaterialTheme.colorScheme.error,
                        modifier = Modifier.padding(16.dp)
                    )
                }
            }
        }
    }
}
