pipeline {
    agent none

    environment {
        BOARD_NAME = "microspora"
        APP_DIR = 'app'
    }

    stages {
        stage('Build in Docker') {
            agent {
                dockerfile {
                    filename '.devcontainer/Dockerfile.ci'
                    args '-u root:root'
                }
            }
            steps {
                checkout scm
                sh '''
                    echo "=== Preparing Zephyr workspace ==="
                    west init -l app || true
                    west update
                    west zephyr-export

                    echo "=== Building application ==="
                    ls -al
                    export ZEPHYR_BASE=$PWD/zephyr
                    west build -p -b ${BOARD_NAME} $PWD/${APP_DIR} -DBOARD_ROOT=$PWD
                '''
                archiveArtifacts artifacts: "build/zephyr/zephyr.elf", fingerprint: true
            }
        }
    }

    post {
        failure {
            echo 'Build failed.'
        }
    }
}
