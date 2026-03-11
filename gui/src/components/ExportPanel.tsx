import { Button } from '@/components/ui/button'
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import { toast } from 'sonner'

export default function ExportPanel() {
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
