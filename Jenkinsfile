pipeline {
    agent {
        dockerfile {
            filename '.devcontainer/Dockerfile.ci'
        }
    }

    environment {
        BOARD_NAME = "stm32mp157c_dk2"
        APP_DIR = 'app'
    }

    stages {
        stage('Checkout') {
            steps {
                sh 'ls -al'
            }
        }

        stage('Build') {
            steps {
                sh '''
                west build -p -b ${BOARD_NAME} ${APP_DIR}
                '''
            }
        }

        stage('Archive Artifacts') {
            steps {
                archiveArtifacts artifacts: 'build/zephyr/zephyr.elf', fingerprint: true
            }
        }
    }

    post {
        failure {
            echo 'Build failed.'
        }
    }
}
