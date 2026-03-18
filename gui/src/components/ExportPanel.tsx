import { useState } from 'react'
import { Button } from '@/components/ui/button'
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import { toast } from 'sonner'
import { useStore } from '@/lib/store'

export default function ExportPanel() {
  const outputDir = useStore((s) => s.outputDir)
  const [plotting, setPlotting] = useState(false)

  const handleExport = async (format: string) => {
    toast.success(`Export ${format} triggered. Check the output directory.`)
  }

  const handleOpenFolder = async () => {
    if (window.electronAPI) {
      const dir = await window.electronAPI.selectDirectory()
      if (dir) {
        window.electronAPI.openExternal(dir)
      }
    }
  }

  const handleGeneratePlots = async () => {
    if (!window.electronAPI || !outputDir) return

    const analysisJsonPath = `${outputDir}/analysis.json`
    const plotsDir = `${outputDir}/plots`

    setPlotting(true)
    try {
      const result = await window.electronAPI.generatePlots({ analysisJsonPath, outputDir: plotsDir })
      if (result.success) {
        toast.success('Plots saved to output directory.')
        window.electronAPI.openExternal(plotsDir)
      } else {
        toast.error(result.error ?? 'Plot generation failed')
      }
    } catch (err) {
      toast.error(err instanceof Error ? err.message : 'Plot generation failed')
    } finally {
      setPlotting(false)
    }
  }

  return (
    <Card>
      <CardHeader className="pb-2">
        <CardTitle className="text-base">Export Results</CardTitle>
      </CardHeader>
      <CardContent>
        <div className="flex flex-wrap gap-2">
          <Button variant="outline" size="sm" onClick={() => handleExport('JSON')}>
            Export JSON
          </Button>
          <Button variant="outline" size="sm" onClick={() => handleExport('CSV')}>
            Export CSV
          </Button>
          <Button variant="outline" size="sm" onClick={() => handleExport('All')}>
            Export All
          </Button>
          {window.electronAPI && outputDir && (
            <Button
              variant="outline"
              size="sm"
              onClick={handleGeneratePlots}
              disabled={plotting}
            >
              {plotting ? 'Generating...' : 'Generate Plots'}
            </Button>
          )}
          {window.electronAPI && (
            <Button variant="ghost" size="sm" onClick={handleOpenFolder}>
              Open Folder
            </Button>
          )}
        </div>
      </CardContent>
    </Card>
  )
}
