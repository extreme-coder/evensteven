import { defineConfig, type Plugin } from 'vite'
import react from '@vitejs/plugin-react'
import tailwindcss from '@tailwindcss/vite'
import electron from 'vite-plugin-electron'
import renderer from 'vite-plugin-electron-renderer'
import path from 'node:path'
import fs from 'node:fs'

function copyPreload(): Plugin {
  return {
    name: 'copy-preload',
    buildStart() {
      const src = path.resolve(__dirname, 'electron/preload.cjs')
      const dest = path.resolve(__dirname, 'dist-electron/preload.cjs')
      fs.mkdirSync(path.dirname(dest), { recursive: true })
      fs.copyFileSync(src, dest)
    },
  }
}

export default defineConfig({
  plugins: [
    react(),
    tailwindcss(),
    copyPreload(),
    electron([
      {
        entry: 'electron/main.ts',
        vite: {
          define: {
            __dirname: 'import.meta.dirname',
          },
        },
      },
    ]),
    renderer(),
  ],
  resolve: {
    alias: {
      '@': path.resolve(__dirname, './src'),
    },
  },
})
