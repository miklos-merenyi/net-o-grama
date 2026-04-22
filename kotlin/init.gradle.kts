// Init script to work around Java 26 parsing issues in Kotlin DSL
beforeSettings { settings ->
    // Workaround for Java 26 version parsing
    System.setProperty("gradle.kotlin.dsl.preventivelyDisableKotlinDslScriptCompilation", "false")
}
