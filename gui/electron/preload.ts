import { contextBridge, ipcRenderer } from 'electron'

export type ProgressData = {
  stage: string
  percent: number
  song?: string
}

const api = {
  analyzeFiles: (args: {
    files: string[]
    projectName: string
    outputDir?: string
    configPath?: string
  }) => ipcRenderer.invoke('analyze-files', args),

  onProgress: (callback: (data: ProgressData) => void) => {
    const handler = (_event: Electron.IpcRendererEvent, data: ProgressData) => callback(data)
    ipcRenderer.on('analysis-progress', handler)
    return () => ipcRenderer.removeListener('analysis-progress', handler)
  },

  cancelAnalysis: () => ipcRenderer.invoke('cancel-analysis'),

  probeFiles: (paths: string[]) => ipcRenderer.invoke('probe-files', paths) as Promise<{
    filename: string
    duration_s: number
    sample_rate: number
    channels: number
    format: string
    error?: string
  }[]>,

  selectFiles: () => ipcRenderer.invoke('select-files') as Promise<string[]>,

  selectDirectory: () => ipcRenderer.invoke('select-directory') as Promise<string | null>,

  loadDemoData: () => ipcRenderer.invoke('load-demo-data'),

  saveProject: (project: unknown) =>
    ipcRenderer.invoke('save-project', { project }),

  loadProject: () => ipcRenderer.invoke('load-project'),

  openExternal: (path: string) => ipcRenderer.invoke('open-external', path),
  generatePlots: (args: { analysisJsonPath: string, outputDir: string }) =>
    ipcRenderer.invoke('generate-plots', args) as Promise<{ success: boolean, outputDir?: string, error?: string }>,
}

contextBridge.exposeInMainWorld('electronAPI', api)

declare global {
  interface Window {
    electronAPI: typeof api
  }
}
