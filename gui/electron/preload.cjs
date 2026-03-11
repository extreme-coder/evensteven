const { contextBridge, ipcRenderer } = require('electron')

const api = {
  analyzeFiles: (args) => ipcRenderer.invoke('analyze-files', args),

  onProgress: (callback) => {
    const handler = (_event, data) => callback(data)
    ipcRenderer.on('analysis-progress', handler)
    return () => ipcRenderer.removeListener('analysis-progress', handler)
  },

  cancelAnalysis: () => ipcRenderer.invoke('cancel-analysis'),
  probeFiles: (paths) => ipcRenderer.invoke('probe-files', paths),
  selectFiles: () => ipcRenderer.invoke('select-files'),
  selectDirectory: () => ipcRenderer.invoke('select-directory'),
  loadDemoData: () => ipcRenderer.invoke('load-demo-data'),
  saveProject: (project) => ipcRenderer.invoke('save-project', { project }),
  loadProject: () => ipcRenderer.invoke('load-project'),
  openExternal: (path) => ipcRenderer.invoke('open-external', path),
}

contextBridge.exposeInMainWorld('electronAPI', api)
