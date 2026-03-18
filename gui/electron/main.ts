import { app, BrowserWindow, ipcMain, dialog, shell } from 'electron'
import path from 'node:path'
import { EngineProcess, getEnginePath } from './engine'
import { execFile, execFileSync } from 'node:child_process'
import fs from 'node:fs'

function findExecutable(candidates: string[]): string | null {
  for (const c of candidates) {
    // Absolute path — just check it exists
    if (path.isAbsolute(c)) {
      if (fs.existsSync(c)) return c
      continue
    }
    // Bare name — try which/where
    try {
      const cmd = process.platform === 'win32' ? 'where' : 'which'
      const result = execFileSync(cmd, [c], { timeout: 3000, stdio: ['ignore', 'pipe', 'ignore'] })
      const resolved = result.toString().trim().split('\n')[0]
      if (resolved && fs.existsSync(resolved)) return resolved
    } catch { /* not found */ }
  }
  return null
}

function findMatlab(): string | null {
  return findExecutable(
    process.platform === 'win32'
      ? ['matlab.exe']
      : ['/Applications/MATLAB.app/bin/matlab', '/usr/local/bin/matlab', 'matlab']
  )
}

function findOctave(): string | null {
  return findExecutable(
    process.platform === 'win32'
      ? ['octave-cli.exe', 'octave.exe']
      : ['/opt/homebrew/bin/octave', '/usr/local/bin/octave', 'octave-cli', 'octave']
  )
}

let mainWindow: BrowserWindow | null = null
let currentEngine: EngineProcess | null = null

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1280,
    height: 860,
    minWidth: 900,
    minHeight: 600,
    webPreferences: {
      preload: path.join(__dirname, 'preload.cjs'),
      contextIsolation: true,
      nodeIntegration: false,
    },
    titleBarStyle: 'hiddenInset',
    title: 'EvenSteven',
  })

  if (process.env.VITE_DEV_SERVER_URL) {
    mainWindow.loadURL(process.env.VITE_DEV_SERVER_URL)
    mainWindow.webContents.openDevTools()
  } else {
    mainWindow.loadFile(path.join(__dirname, '../dist/index.html'))
  }
}

app.whenReady().then(createWindow)

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit()
})

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) createWindow()
})

// IPC Handlers

ipcMain.handle('select-files', async () => {
  if (!mainWindow) return []
  const result = await dialog.showOpenDialog(mainWindow, {
    properties: ['openFile', 'multiSelections'],
    filters: [
      { name: 'Audio Files', extensions: ['wav', 'flac', 'mp3'] },
    ],
  })
  return result.filePaths
})

ipcMain.handle('probe-files', async (_event, filePaths: string[]) => {
  const enginePath = getEnginePath()
  if (!fs.existsSync(enginePath)) {
    return filePaths.map((p) => ({
      filename: path.basename(p),
      duration_s: 0,
      sample_rate: 0,
      channels: 0,
      format: path.extname(p).slice(1).toLowerCase(),
    }))
  }

  return new Promise((resolve) => {
    execFile(
      enginePath,
      ['--probe', ...filePaths],
      { timeout: 10000 },
      (error: Error | null, stdout: string) => {
        if (error || !stdout.trim()) {
          resolve(filePaths.map((p) => ({
            filename: path.basename(p),
            duration_s: 0,
            sample_rate: 0,
            channels: 0,
            format: path.extname(p).slice(1).toLowerCase(),
          })))
          return
        }
        try {
          resolve(JSON.parse(stdout.trim()))
        } catch {
          resolve(filePaths.map((p) => ({
            filename: path.basename(p),
            duration_s: 0,
            sample_rate: 0,
            channels: 0,
            format: path.extname(p).slice(1).toLowerCase(),
          })))
        }
      }
    )
  })
})

ipcMain.handle('select-directory', async () => {
  if (!mainWindow) return null
  const result = await dialog.showOpenDialog(mainWindow, {
    properties: ['openDirectory', 'createDirectory'],
  })
  return result.filePaths[0] ?? null
})

ipcMain.handle('analyze-files', async (_event, args: {
  files: string[]
  projectName: string
  outputDir?: string
  configPath?: string
}) => {
  if (currentEngine) {
    currentEngine.kill()
  }

  const enginePath = getEnginePath()
  const outputDir = args.outputDir ?? path.join(app.getPath('temp'), 'evensteven-output')

  const engineArgs = [
    '--progress',
    '--project', args.projectName,
    '--output', outputDir,
    '--format', 'both',
    ...args.files,
  ]

  if (args.configPath) {
    engineArgs.unshift('--config', args.configPath)
  }

  currentEngine = new EngineProcess(enginePath, engineArgs)

  currentEngine.onProgress((data) => {
    mainWindow?.webContents.send('analysis-progress', data)
  })

  try {
    const result = await currentEngine.run()
    currentEngine = null

    // Read the analysis.json
    const analysisPath = path.join(outputDir, 'analysis.json')
    if (fs.existsSync(analysisPath)) {
      const json = fs.readFileSync(analysisPath, 'utf-8')
      return { success: true, data: JSON.parse(json), outputDir }
    }

    return result
  } catch (err: unknown) {
    currentEngine = null
    const message = err instanceof Error ? err.message : String(err)
    return { success: false, error: message }
  }
})

ipcMain.handle('cancel-analysis', () => {
  if (currentEngine) {
    currentEngine.kill()
    currentEngine = null
  }
})

ipcMain.handle('load-demo-data', async () => {
  const enginePath = getEnginePath()
  const outputDir = path.join(app.getPath('temp'), 'evensteven-demo')

  // Try running engine with --demo
  if (fs.existsSync(enginePath)) {
    const engine = new EngineProcess(enginePath, [
      '--demo', '--progress', '--output', outputDir, '--format', 'both',
    ])

    engine.onProgress((data) => {
      mainWindow?.webContents.send('analysis-progress', data)
    })

    try {
      await engine.run()
      const analysisPath = path.join(outputDir, 'analysis.json')
      if (fs.existsSync(analysisPath)) {
        const json = fs.readFileSync(analysisPath, 'utf-8')
        return { success: true, data: JSON.parse(json), outputDir }
      }
    } catch {
      // Fall through to fallback
    }
  }

  return { success: false, error: `C++ engine not found at ${enginePath}. Build it with: cd cpp && cmake -B build && cmake --build build` }
})

ipcMain.handle('open-external', (_event, dirPath: string) => {
  shell.openPath(dirPath)
})

ipcMain.handle('save-project', async (_event, data: { path?: string, project: unknown }) => {
  let savePath = data.path
  if (!savePath && mainWindow) {
    const result = await dialog.showSaveDialog(mainWindow, {
      defaultPath: 'project.evensteven.json',
      filters: [{ name: 'EvenSteven Project', extensions: ['evensteven.json'] }],
    })
    savePath = result.filePath
  }
  if (savePath) {
    fs.writeFileSync(savePath, JSON.stringify(data.project, null, 2))
    return { success: true, path: savePath }
  }
  return { success: false }
})

ipcMain.handle('load-project', async () => {
  if (!mainWindow) return null
  const result = await dialog.showOpenDialog(mainWindow, {
    filters: [{ name: 'EvenSteven Project', extensions: ['evensteven.json'] }],
  })
  if (result.filePaths.length > 0) {
    const json = fs.readFileSync(result.filePaths[0], 'utf-8')
    return JSON.parse(json)
  }
  return null
})

ipcMain.handle('generate-plots', async (_event, args: { analysisJsonPath: string, outputDir: string }) => {
  const matlabDir = app.isPackaged
    ? path.join(process.resourcesPath, 'matlab')
    : path.join(__dirname, '../../matlab')

  const runnerScript = path.join(matlabDir, 'generate_all_plots.m')
  if (!fs.existsSync(runnerScript)) {
    return { success: false, error: 'MATLAB scripts not found' }
  }

  const matlab = findMatlab()
  const octave = findOctave()

  if (!matlab && !octave) {
    return { success: false, error: 'Neither MATLAB nor Octave found. Install Octave (brew install octave) or MATLAB to generate plots.' }
  }

  const jsonPath = args.analysisJsonPath.replace(/'/g, "''")
  const outDir = args.outputDir.replace(/'/g, "''")
  const matlabCmd = `addpath('${matlabDir.replace(/'/g, "''")}'); generate_all_plots('${jsonPath}', '${outDir}'); exit;`

  return new Promise((resolve) => {
    if (matlab) {
      execFile(matlab, ['-batch', matlabCmd], { timeout: 120000 }, (err) => {
        if (err) {
          resolve({ success: false, error: `MATLAB failed: ${err.message}` })
        } else {
          resolve({ success: true, outputDir: args.outputDir })
        }
      })
    } else if (octave) {
      execFile(octave, ['--no-gui', '--eval', matlabCmd], { timeout: 120000 }, (err) => {
        if (err) {
          resolve({ success: false, error: `Octave failed: ${err.message}` })
        } else {
          resolve({ success: true, outputDir: args.outputDir })
        }
      })
    }
  })
})
