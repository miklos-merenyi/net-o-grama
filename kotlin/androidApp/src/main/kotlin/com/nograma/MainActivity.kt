package com.nograma

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.animation.*
import androidx.compose.animation.core.tween
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.LazyRow
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
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
    var serverConfirmedConnected by remember { mutableStateOf(false) }
    var playerName by remember { mutableStateOf("") }
    var serverAddress by remember { mutableStateOf("192.168.1.18") }
    var serverPort by remember { mutableStateOf("5555") }
    var currentAnswer by remember { mutableStateOf("") }
    var isConnecting by remember { mutableStateOf(false) }
    var errorMessage by remember { mutableStateOf("") }
    val coroutineScope = rememberCoroutineScope()
    
    MaterialTheme {
        Surface(
            modifier = Modifier
                .fillMaxSize()
                .background(
                    brush = Brush.verticalGradient(
                        colors = listOf(
                            MaterialTheme.colorScheme.background,
                            MaterialTheme.colorScheme.background.copy(alpha = 0.95f)
                        )
                    )
                )
        ) {
            AnimatedContent(
                targetState = gameClient == null,
                transitionSpec = {
                    fadeIn(animationSpec = tween(300)) + scaleIn(animationSpec = tween(300)) togetherWith
                    fadeOut(animationSpec = tween(300)) + scaleOut(animationSpec = tween(300))
                },
                label = "screenTransition"
            ) { isConnectionScreen ->
                when {
                    isConnectionScreen || !serverConfirmedConnected -> ConnectionScreen(activity, playerName, serverAddress, serverPort, isConnecting, errorMessage, { playerName = it }, { serverAddress = it }, { serverPort = it }, { errorMessage = "" }) { host, port, name ->
                        errorMessage = ""
                        isConnecting = true
                        serverConfirmedConnected = false
                        val client = createGameClient(host, port, name)
                        
                        // Set up all listeners BEFORE connecting to catch early responses
                        client.setGameStateListener { newState ->
                            gameState = newState
                        }
                        client.setErrorListener { error ->
                            errorMessage = error
                            isConnecting = false
                        }
                        client.setConnectedConfirmedListener {
                            serverConfirmedConnected = true
                            isConnecting = false
                            errorMessage = ""
                        }
                        client.setGuessRejectedListener {
                            currentAnswer = ""
                        }
                        client.setGuessAcceptedListener {
                            currentAnswer = ""
                        }
                        
                        gameClient = client
                        coroutineScope.launch {
                            try {
                                client.connect(name)
                            } catch (e: Exception) {
                                errorMessage = "Failed to connect: ${e.message}"
                                gameClient = null
                                serverConfirmedConnected = false
                                isConnecting = false
                            }
                        }
                    }
                    gameState.currentPhase == GamePhase.WAITING -> WaitingScreen(gameClient) {
                        gameClient?.disconnect()
                        gameClient = null
                        serverConfirmedConnected = false
                        isConnecting = false
                        errorMessage = ""
                    }
                    gameState.currentPhase == GamePhase.COUNTDOWN -> CountdownScreen(gameState)
                    gameState.currentPhase == GamePhase.PLAYING -> PlayingScreen(gameState, currentAnswer, { currentAnswer = it }, { gameClient?.submitWord(it) }, { gameClient?.shuffle() }, { gameClient?.disconnect(); gameClient = null; serverConfirmedConnected = false; isConnecting = false; currentAnswer = ""; errorMessage = "" })
                }
            }
            
            if (errorMessage.isNotEmpty() && !serverConfirmedConnected) {
                Box(
                    modifier = Modifier
                        .fillMaxSize()
                        .padding(16.dp),
                    contentAlignment = Alignment.BottomCenter
                ) {
                    Card(
                        modifier = Modifier
                            .fillMaxWidth()
                            .clip(RoundedCornerShape(12.dp)),
                        colors = CardDefaults.cardColors(
                            containerColor = MaterialTheme.colorScheme.errorContainer
                        )
                    ) {
                        Text(
                            errorMessage,
                            color = MaterialTheme.colorScheme.error,
                            modifier = Modifier.padding(16.dp),
                            fontSize = 14.sp
                        )
                    }
                }
            }
        }
    }
}

@Composable
fun ConnectionScreen(
    activity: MainActivity,
    playerName: String,
    serverAddress: String,
    serverPort: String,
    isConnecting: Boolean,
    errorMessage: String,
    onPlayerNameChange: (String) -> Unit,
    onServerAddressChange: (String) -> Unit,
    onServerPortChange: (String) -> Unit,
    onErrorClear: () -> Unit,
    onConnect: (String, Int, String) -> Unit
) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(horizontal = 24.dp, vertical = 32.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        // Connection form card
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .clip(RoundedCornerShape(20.dp)),
            colors = CardDefaults.cardColors(
                containerColor = MaterialTheme.colorScheme.surface
            ),
            elevation = CardDefaults.cardElevation(defaultElevation = 8.dp)
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(24.dp),
                verticalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                TextField(
                    value = serverAddress,
                    onValueChange = onServerAddressChange,
                    label = { Text("Server Address") },
                    modifier = Modifier
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(12.dp)),
                    placeholder = { Text("e.g., 192.168.1.100") },
                    leadingIcon = { Icon(Icons.Filled.Settings, null) },
                    singleLine = true,
                    colors = TextFieldDefaults.colors()
                )
                
                TextField(
                    value = serverPort,
                    onValueChange = onServerPortChange,
                    label = { Text("Server Port") },
                    modifier = Modifier
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(12.dp)),
                    placeholder = { Text("e.g., 5555") },
                    leadingIcon = { Icon(Icons.Filled.Settings, null) },
                    singleLine = true,
                    keyboardOptions = KeyboardOptions(imeAction = ImeAction.Next),
                    colors = TextFieldDefaults.colors()
                )
                
                TextField(
                    value = playerName,
                    onValueChange = onPlayerNameChange,
                    label = { Text("Player Name") },
                    modifier = Modifier
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(12.dp)),
                    placeholder = { Text("Enter your name") },
                    leadingIcon = { Icon(Icons.Filled.Person, null) },
                    singleLine = true,
                    keyboardOptions = KeyboardOptions(imeAction = ImeAction.Done),
                    colors = TextFieldDefaults.colors()
                )
                
                Spacer(modifier = Modifier.height(8.dp))
                
                Button(
                    onClick = {
                        val port = serverPort.toIntOrNull() ?: 5555
                        onConnect(serverAddress, port, playerName)
                    },
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(52.dp)
                        .clip(RoundedCornerShape(12.dp)),
                    enabled = !isConnecting && serverAddress.isNotEmpty() && playerName.isNotEmpty(),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MaterialTheme.colorScheme.primary
                    )
                ) {
                    if (isConnecting) {
                        CircularProgressIndicator(
                            modifier = Modifier.size(20.dp),
                            color = MaterialTheme.colorScheme.onPrimary,
                            strokeWidth = 2.dp
                        )
                        Spacer(modifier = Modifier.width(12.dp))
                    }
                    Text(if (isConnecting) "Connecting..." else "Join Game", fontSize = 16.sp, fontWeight = FontWeight.SemiBold)
                }
            }
        }
        
        Spacer(modifier = Modifier.height(16.dp))
        
        Button(
            onClick = { activity.finish() },
            modifier = Modifier
                .fillMaxWidth()
                .height(48.dp)
                .clip(RoundedCornerShape(12.dp)),
            colors = ButtonDefaults.buttonColors(
                containerColor = MaterialTheme.colorScheme.errorContainer
            )
        ) {
            Icon(Icons.Filled.Close, null, tint = MaterialTheme.colorScheme.error)
            Spacer(modifier = Modifier.width(8.dp))
            Text("Exit", color = MaterialTheme.colorScheme.error, fontWeight = FontWeight.SemiBold)
        }
    }
}

@Composable
fun WaitingScreen(
    gameClient: GameClient?,
    onDisconnect: () -> Unit
) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(24.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        Card(
            modifier = Modifier
                .fillMaxWidth(0.8f)
                .clip(RoundedCornerShape(20.dp)),
            colors = CardDefaults.cardColors(
                containerColor = MaterialTheme.colorScheme.primaryContainer
            ),
            elevation = CardDefaults.cardElevation(defaultElevation = 12.dp)
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(32.dp),
                horizontalAlignment = Alignment.CenterHorizontally,
                verticalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                Icon(
                    Icons.Filled.CheckCircle,
                    contentDescription = null,
                    modifier = Modifier.size(64.dp),
                    tint = MaterialTheme.colorScheme.primary
                )
                
                Text(
                    "Connected!",
                    fontSize = 28.sp,
                    fontWeight = FontWeight.Bold,
                    color = MaterialTheme.colorScheme.primary
                )
                
                Text(
                    "Waiting for other players...",
                    fontSize = 16.sp,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    textAlign = TextAlign.Center
                )
                
                CircularProgressIndicator(
                    modifier = Modifier.size(40.dp),
                    color = MaterialTheme.colorScheme.primary,
                    strokeWidth = 3.dp
                )
            }
        }
        
        Spacer(modifier = Modifier.height(32.dp))
        
        Button(
            onClick = { gameClient?.markReady() },
            modifier = Modifier
                .fillMaxWidth(0.7f)
                .height(52.dp)
                .clip(RoundedCornerShape(12.dp)),
            colors = ButtonDefaults.buttonColors(
                containerColor = MaterialTheme.colorScheme.primary
            )
        ) {
            Icon(Icons.Filled.ThumbUp, null, tint = MaterialTheme.colorScheme.onPrimary)
            Spacer(modifier = Modifier.width(8.dp))
            Text("Ready to Play", fontSize = 16.sp, fontWeight = FontWeight.SemiBold)
        }
        
        Spacer(modifier = Modifier.height(12.dp))
        
        OutlinedButton(
            onClick = onDisconnect,
            modifier = Modifier
                .fillMaxWidth(0.7f)
                .height(48.dp)
                .clip(RoundedCornerShape(12.dp))
        ) {
            Icon(Icons.Filled.Close, null, tint = MaterialTheme.colorScheme.outline)
            Spacer(modifier = Modifier.width(8.dp))
            Text("Disconnect", color = MaterialTheme.colorScheme.outline)
        }
    }
}

@Composable
fun CountdownScreen(gameState: GameState) {
    Box(
        modifier = Modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        Column(
            modifier = Modifier.fillMaxSize(),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Text(
                "Game Starting",
                fontSize = 32.sp,
                fontWeight = FontWeight.SemiBold,
                color = MaterialTheme.colorScheme.primary,
                modifier = Modifier.padding(bottom = 32.dp)
            )
            
            if (gameState.countdownMessage.isNotEmpty()) {
                AnimatedContent(
                    targetState = gameState.countdownMessage,
                    transitionSpec = {
                        fadeIn(animationSpec = tween(300)) + scaleIn(animationSpec = tween(300)) togetherWith
                        fadeOut(animationSpec = tween(300)) + scaleOut(animationSpec = tween(300))
                    },
                    label = "countdownTransition"
                ) { message ->
                    val (color, size) = when (message) {
                        "READY" -> Color(0xFFFFD700) to 88.sp
                        "STEADY" -> Color(0xFFFF1493) to 88.sp
                        "GO!" -> Color(0xFF00DD00) to 100.sp
                        else -> MaterialTheme.colorScheme.primary to 72.sp
                    }
                    
                    Text(
                        message,
                        fontSize = size,
                        fontWeight = FontWeight.ExtraBold,
                        color = color,
                        textAlign = TextAlign.Center,
                        modifier = Modifier.padding(32.dp)
                    )
                }
            } else {
                Text(
                    "Get ready!",
                    fontSize = 24.sp,
                    color = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.padding(16.dp)
                )
            }
        }
    }
}

@OptIn(ExperimentalLayoutApi::class)
@Composable
fun PlayingScreen(
    gameState: GameState,
    currentAnswer: String,
    onAnswerChange: (String) -> Unit,
    onSubmitWord: (String) -> Unit,
    onShuffle: () -> Unit,
    onQuit: () -> Unit
) {
    val usedLetterIndices = remember(gameState.shuffledWord, currentAnswer) {
        val usedIndices = mutableSetOf<Int>()
        val tempGuess = currentAnswer.toMutableList()
        gameState.shuffledWord.forEachIndexed { index, letter ->
            val guessIndex = tempGuess.indexOf(letter)
            if (guessIndex >= 0) {
                usedIndices.add(index)
                tempGuess.removeAt(guessIndex)
            }
        }
        usedIndices
    }
    var showQuitConfirm by remember { mutableStateOf(false) }

    Column(
        modifier = Modifier.fillMaxSize()
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 8.dp, vertical = 4.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            if (gameState.wordSlots.isNotEmpty()) {
                Text(
                    "${gameState.wordSlots.count { it.playerIndex != -1 }}/${gameState.wordSlots.size}",
                    fontSize = 13.sp,
                    fontWeight = FontWeight.Normal,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    modifier = Modifier.weight(1f).padding(start = 8.dp)
                )
            } else {
                Spacer(modifier = Modifier.weight(1f))
            }
            Text(
                "${gameState.timeRemaining}s",
                fontSize = 16.sp,
                fontWeight = FontWeight.Bold,
                color = MaterialTheme.colorScheme.primary
            )
            IconButton(onClick = { showQuitConfirm = true }, modifier = Modifier.size(32.dp)) {
                Icon(
                    Icons.Filled.Close,
                    contentDescription = "Quit",
                    tint = MaterialTheme.colorScheme.error,
                    modifier = Modifier.size(20.dp)
                )
            }
        }
        
        Column(
            modifier = Modifier
                .weight(1f)
                .verticalScroll(rememberScrollState())
                .padding(horizontal = 12.dp, vertical = 12.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            if (gameState.players.isNotEmpty()) {
                val playerColors = listOf(
                    Color(0xFFFF6B6B),
                    Color(0xFF51CF66),
                    Color(0xFFFFD93D),
                    Color(0xFF6BCFDF)
                )
                val rows = gameState.players.chunked(2)
                Column(
                    modifier = Modifier.fillMaxWidth(),
                    verticalArrangement = Arrangement.spacedBy(4.dp)
                ) {
                    rows.forEach { rowPlayers ->
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.spacedBy(8.dp)
                        ) {
                            rowPlayers.forEach { player ->
                                val idx = gameState.players.indexOf(player)
                                val playerColor = playerColors.getOrElse(idx) { MaterialTheme.colorScheme.primary }
                                Row(
                                    modifier = Modifier
                                        .weight(1f)
                                        .clip(RoundedCornerShape(8.dp))
                                        .background(MaterialTheme.colorScheme.surfaceVariant)
                                        .padding(horizontal = 8.dp, vertical = 6.dp),
                                    verticalAlignment = Alignment.CenterVertically,
                                    horizontalArrangement = Arrangement.SpaceBetween
                                ) {
                                    Text(
                                        player.name,
                                        fontSize = 13.sp,
                                        fontWeight = FontWeight.SemiBold,
                                        color = playerColor,
                                        maxLines = 1,
                                        overflow = TextOverflow.Ellipsis,
                                        modifier = Modifier.weight(1f)
                                    )
                                    Text(
                                        "${player.score}",
                                        fontSize = 13.sp,
                                        fontWeight = FontWeight.Bold,
                                        color = MaterialTheme.colorScheme.onSurface
                                    )
                                }
                            }
                            // pad if odd number of players
                            if (rowPlayers.size == 1) {
                                Spacer(modifier = Modifier.weight(1f))
                            }
                        }
                    }
                }
            }

            if (gameState.wordSlots.isNotEmpty()) {
                val sortedSlots = gameState.wordSlots.sortedBy { it.length }
                val colSize = maxOf(3, (sortedSlots.size + 3) / 4)
                val columns = sortedSlots.chunked(colSize)

                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(14.dp))
                        .background(MaterialTheme.colorScheme.surfaceVariant)
                        .padding(8.dp),
                    horizontalArrangement = Arrangement.spacedBy(6.dp)
                ) {
                    columns.forEach { slotsInGroup ->
                        val colWeight = (slotsInGroup.maxOfOrNull { it.length } ?: 1).toFloat()
                        Column(
                            modifier = Modifier.weight(colWeight),
                            verticalArrangement = Arrangement.spacedBy(4.dp)
                        ) {
                            slotsInGroup.forEach { slot ->
                                val playerColor = if (slot.playerIndex == -1) {
                                    MaterialTheme.colorScheme.onSurfaceVariant
                                } else {
                                    when (slot.playerIndex) {
                                        0 -> Color(0xFFFF6B6B)
                                        1 -> Color(0xFF51CF66)
                                        2 -> Color(0xFFFFD93D)
                                        3 -> Color(0xFF6BCFDF)
                                        else -> MaterialTheme.colorScheme.onSurfaceVariant
                                    }
                                }

                                Card(
                                    modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(8.dp)),
                                    colors = CardDefaults.cardColors(
                                        containerColor = if (slot.playerIndex == -1) MaterialTheme.colorScheme.surface else MaterialTheme.colorScheme.primaryContainer
                                    ),
                                    elevation = CardDefaults.cardElevation(
                                        defaultElevation = if (slot.playerIndex == -1) 0.dp else 4.dp
                                    )
                                ) {
                                    Text(
                                        slot.word,
                                        fontSize = 13.sp,
                                        fontWeight = if (slot.playerIndex == -1) FontWeight.Normal else FontWeight.Bold,
                                        modifier = Modifier.padding(horizontal = 6.dp, vertical = 5.dp).fillMaxWidth(),
                                        color = playerColor,
                                        textAlign = TextAlign.Center,
                                        fontFamily = androidx.compose.ui.text.font.FontFamily.Monospace,
                                        maxLines = 1
                                    )
                                }
                            }
                        }
                    }
                }
            }

            Spacer(modifier = Modifier.height(8.dp))
        }

        Card(
            modifier = Modifier
                .fillMaxWidth()
                .clip(RoundedCornerShape(18.dp)),
            colors = CardDefaults.cardColors(
                containerColor = MaterialTheme.colorScheme.surface
            ),
            elevation = CardDefaults.cardElevation(defaultElevation = 8.dp)
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(14.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    val guessCapacity = maxOf(gameState.shuffledWord.length, 1)
                    repeat(guessCapacity) { index ->
                        val guessChar = currentAnswer.getOrNull(index)?.toString() ?: ""
                        OutlinedButton(
                            onClick = {
                                if (index < currentAnswer.length) {
                                    onAnswerChange(
                                        buildString {
                                            append(currentAnswer.substring(0, index))
                                            append(currentAnswer.substring(index + 1))
                                        }
                                    )
                                }
                            },
                            modifier = Modifier
                                .weight(1f)
                                .height(52.dp),
                            shape = RoundedCornerShape(12.dp),
                            contentPadding = PaddingValues(0.dp),
                            border = BorderStroke(1.dp, MaterialTheme.colorScheme.outline)
                        ) {
                            Text(
                                text = if (guessChar.isEmpty()) "_" else guessChar,
                                fontSize = 20.sp,
                                fontWeight = FontWeight.Bold,
                                color = if (guessChar.isEmpty()) MaterialTheme.colorScheme.outline else MaterialTheme.colorScheme.primary,
                                fontFamily = androidx.compose.ui.text.font.FontFamily.Monospace
                            )
                        }
                    }
                }

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    gameState.shuffledWord.forEachIndexed { index, letter ->
                        val isUsed = usedLetterIndices.contains(index)
                        Button(
                            onClick = {
                                if (isUsed) {
                                    onAnswerChange(
                                        buildString {
                                            val charList = currentAnswer.toMutableList()
                                            val removeIndex = charList.indexOf(letter)
                                            if (removeIndex >= 0) {
                                                charList.removeAt(removeIndex)
                                            }
                                            append(charList.joinToString(""))
                                        }
                                    )
                                } else {
                                    onAnswerChange(currentAnswer + letter)
                                }
                            },
                            modifier = Modifier
                                .weight(1f)
                                .height(44.dp),
                            shape = RoundedCornerShape(10.dp),
                            contentPadding = PaddingValues(0.dp),
                            colors = ButtonDefaults.buttonColors(
                                containerColor = if (isUsed) MaterialTheme.colorScheme.surfaceVariant else MaterialTheme.colorScheme.secondaryContainer,
                                contentColor = if (isUsed) MaterialTheme.colorScheme.outline else MaterialTheme.colorScheme.onSecondaryContainer,
                                disabledContainerColor = MaterialTheme.colorScheme.surfaceVariant,
                                disabledContentColor = MaterialTheme.colorScheme.outline
                            )
                        ) {
                            Text(
                                text = letter.toString(),
                                fontSize = 24.sp,
                                fontWeight = FontWeight.Bold,
                                fontFamily = androidx.compose.ui.text.font.FontFamily.Monospace
                            )
                        }
                    }
                }

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    OutlinedButton(
                        onClick = {
                            if (currentAnswer.isNotEmpty()) {
                                onAnswerChange(currentAnswer.dropLast(1))
                            }
                        },
                        modifier = Modifier
                            .weight(1f)
                            .height(46.dp),
                        enabled = currentAnswer.isNotEmpty(),
                        shape = RoundedCornerShape(10.dp)
                    ) {
                        Icon(Icons.Filled.Close, null, modifier = Modifier.size(18.dp))
                        Spacer(modifier = Modifier.width(6.dp))
                        Text("Back", fontSize = 13.sp)
                    }

                    Button(
                        onClick = onShuffle,
                        modifier = Modifier
                            .weight(1f)
                            .height(46.dp)
                            .clip(RoundedCornerShape(10.dp)),
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MaterialTheme.colorScheme.secondary
                        )
                    ) {
                        Icon(Icons.Filled.Refresh, null, modifier = Modifier.size(18.dp))
                        Spacer(modifier = Modifier.width(6.dp))
                        Text("Shuffle", fontSize = 13.sp)
                    }

                    Button(
                        onClick = {
                            if (currentAnswer.isNotEmpty()) {
                                onSubmitWord(currentAnswer)
                            }
                        },
                        modifier = Modifier
                            .weight(1f)
                            .height(46.dp)
                            .clip(RoundedCornerShape(10.dp)),
                        enabled = currentAnswer.isNotEmpty(),
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MaterialTheme.colorScheme.primary
                        )
                    ) {
                        Icon(Icons.Filled.Send, null, tint = MaterialTheme.colorScheme.onPrimary)
                        Spacer(modifier = Modifier.width(6.dp))
                        Text("Send", fontSize = 13.sp)
                    }
                }
            }
        }
    }

    if (showQuitConfirm) {
        AlertDialog(
            onDismissRequest = { showQuitConfirm = false },
            title = { Text("Quit game?") },
            text = { Text("Are you sure you want to quit this round?") },
            confirmButton = {
                TextButton(
                    onClick = {
                        showQuitConfirm = false
                        onQuit()
                    }
                ) {
                    Text("Quit")
                }
            },
            dismissButton = {
                TextButton(onClick = { showQuitConfirm = false }) {
                    Text("Cancel")
                }
            }
        )
    }
}
