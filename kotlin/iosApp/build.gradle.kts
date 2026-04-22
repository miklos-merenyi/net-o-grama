plugins {
    kotlin("multiplatform")
}

kotlin {
    iosX64()
    iosArm64()
    iosSimulatorArm64()

    targets.withType<org.jetbrains.kotlin.gradle.plugin.mpp.KotlinNativeTarget> {
        binaries.framework {
            baseName = "shared"
            isStatic = true
        }
    }

    sourceSets {
        val iosX64Main by getting
        val iosArm64Main by getting
        val iosSimulatorArm64Main by getting
        val iosMain by creating {
            dependsOn(commonMain.get())
            iosX64Main.dependsOn(this)
            iosArm64Main.dependsOn(this)
            iosSimulatorArm64Main.dependsOn(this)
        }
    }
}

task("packForXcode") {
    doLast {
        val mode = System.getenv("CONFIGURATION") ?: "DEBUG"
        val sdkName = System.getenv("SDK_NAME") ?: "iphonesimulator"
        val targetName = "ios" + if (sdkName.startsWith("iphoneos")) "Arm64" else "SimulatorArm64"
        val framework = kotlin.targets.getByName<org.jetbrains.kotlin.gradle.plugin.mpp.KotlinNativeTarget>(targetName)
            .binaries.getFramework(mode)
        inputs.file(framework.outputFile)
        outputs.dir("$buildDir/xcode-frameworks")

        copy {
            from(framework.outputDirectory)
            into("$buildDir/xcode-frameworks/${framework.outputFile.name}")
        }
    }
}

tasks.register("packForXcodeDebug") {
    dependsOn("packForXcode")
    doFirst {
        System.setProperty("CONFIGURATION", "DEBUG")
    }
}

tasks.register("packForXcodeRelease") {
    dependsOn("packForXcode")
    doFirst {
        System.setProperty("CONFIGURATION", "RELEASE")
    }
}
