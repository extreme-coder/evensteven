import { spawn, type ChildProcess } from 'node:child_process'
import path from 'node:path'
import fs from 'node:fs'

export function getEnginePath(): string {
  // Check packaged location first
  if (process.resourcesPath) {
    const packaged = path.join(process.resourcesPath, 'bin', 'evensteven')
    if (fs.existsSync(packaged)) return packaged
  }

  // Dev mode: check cpp/build
  const devPaths = [
    path.join(__dirname, '..', '..', 'cpp', 'build', 'evensteven'),
    path.join(__dirname, '..', '..', '..', 'cpp', 'build', 'evensteven'),
    path.join(process.cwd(), 'cpp', 'build', 'evensteven'),
  ]

  for (const p of devPaths) {
    if (fs.existsSync(p)) return p
  }

  return 'evensteven' // Hope it's on PATH
}

type ProgressCallback = (data: { stage: string; percent: number; song?: string }) => void

export class EngineProcess {
  private proc: ChildProcess | null = null
  private progressCallbacks: ProgressCallback[] = []
  private enginePath: string
  private args: string[]

  constructor(enginePath: string, args: string[]) {
    this.enginePath = enginePath
    this.args = args
  }

  onProgress(cb: ProgressCallback) {
    this.progressCallbacks.push(cb)
  }

  run(): Promise<{ success: boolean; data?: unknown; error?: string }> {
    return new Promise((resolve, reject) => {
      this.proc = spawn(this.enginePath, this.args, {
        stdio: ['ignore', 'pipe', 'pipe'],
      })

      let stderr = ''
      let lastError = ''

      this.proc.stdout?.on('data', (chunk: Buffer) => {
        const text = chunk.toString()

        // Parse JSON lines
        const lines = text.split('\n')
        for (const line of lines) {
          const trimmed = line.trim()
          if (!trimmed) continue
          try {
            const msg = JSON.parse(trimmed)
            if (msg.progress) {
              for (const cb of this.progressCallbacks) {
                cb(msg.progress)
              }
            }
            if (msg.status === 'error' && msg.message) {
              lastError = msg.message
            }
          } catch {
            // Not JSON, ignore
          }
        }
      })

      this.proc.stderr?.on('data', (chunk: Buffer) => {
        stderr += chunk.toString()
      })

      this.proc.on('close', (code) => {
        if (code === 0) {
          resolve({ success: true })
        } else {
          const errorMsg = lastError || stderr.trim() || `Engine exited with code ${code}`
          reject(new Error(errorMsg))
        }
        this.proc = null
      })

      this.proc.on('error', (err) => {
        reject(new Error(`Failed to start engine: ${err.message}`))
        this.proc = null
      })
    })
  }

  kill() {
    if (this.proc) {
      this.proc.kill('SIGTERM')
      this.proc = null
    }
  }
}
