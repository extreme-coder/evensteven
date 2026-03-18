import { useNavigate } from 'react-router-dom'
import { Button } from '@/components/ui/button'
import { Card, CardHeader, CardTitle, CardDescription, CardContent } from '@/components/ui/card'
import {
  Dialog,
  DialogContent,
  DialogHeader,
  DialogTitle,
  DialogFooter,
} from '@/components/ui/dialog'
import { Input } from '@/components/ui/input'
import { Label } from '@/components/ui/label'
import { useStore } from '@/lib/store'
import { useState } from 'react'

export default function HomePage() {
  const navigate = useNavigate()
  const { setProjectName, setStatus, setResults, setError, reset } = useStore()
  const [dialogOpen, setDialogOpen] = useState(false)
  const [name, setName] = useState('My Setlist')
  const [demoLoading, setDemoLoading] = useState(false)

  const handleNewProject = () => {
    reset()
    setProjectName(name)
    setDialogOpen(false)
    navigate('/project')
  }

  const handleLoadDemo = async () => {
    reset()
    setDemoLoading(true)
    setStatus('running')
    navigate('/analysis')

    try {
      if (!window.electronAPI) {
        setError('Electron API not available. Please run in Electron.')
        return
      }
      const result = await window.electronAPI.loadDemoData()
      if (result.success) {
        setResults(result.data, result.outputDir)
        setStatus('complete')
      } else {
        setError(result.error ?? 'Demo analysis failed')
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Unexpected error during demo analysis')
    } finally {
      setDemoLoading(false)
    }
  }

  return (
    <div className="flex flex-col items-center justify-center min-h-screen p-8">
      <div className="max-w-2xl w-full space-y-8">
        <div className="text-center space-y-3">
          <h1 className="text-5xl font-bold tracking-tight">EvenSteven</h1>
          <p className="text-xl text-muted-foreground">
            Setlist Loudness Balancer
          </p>
          <p className="text-sm text-muted-foreground max-w-md mx-auto">
            Analyze your rehearsal and live recordings for loudness balance.
            Get actionable feedback to make every song in your set sound great.
          </p>
        </div>

        <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
          <Card
            className="cursor-pointer hover:border-primary transition-colors"
            onClick={() => setDialogOpen(true)}
          >
            <CardHeader>
              <CardTitle>New Project</CardTitle>
              <CardDescription>
                Import audio files and analyze your setlist
              </CardDescription>
            </CardHeader>
            <CardContent>
              <Button className="w-full">Create Project</Button>
            </CardContent>
          </Card>

          <Card
            className="cursor-pointer hover:border-primary transition-colors"
            onClick={handleLoadDemo}
          >
            <CardHeader>
              <CardTitle>Load Demo</CardTitle>
              <CardDescription>
                Try EvenSteven with pre-generated sample data
              </CardDescription>
            </CardHeader>
            <CardContent>
              <Button variant="secondary" className="w-full" disabled={demoLoading}>
                {demoLoading ? 'Loading...' : 'Load Demo'}
              </Button>
            </CardContent>
          </Card>
        </div>
      </div>

      <Dialog open={dialogOpen} onOpenChange={setDialogOpen}>
        <DialogContent>
          <DialogHeader>
            <DialogTitle>New Project</DialogTitle>
          </DialogHeader>
          <div className="space-y-4 py-4">
            <div className="space-y-2">
              <Label htmlFor="project-name">Project Name</Label>
              <Input
                id="project-name"
                value={name}
                onChange={(e) => setName(e.target.value)}
                placeholder="My Setlist"
                onKeyDown={(e) => e.key === 'Enter' && handleNewProject()}
              />
            </div>
          </div>
          <DialogFooter>
            <Button variant="outline" onClick={() => setDialogOpen(false)}>
              Cancel
            </Button>
            <Button onClick={handleNewProject}>Create</Button>
          </DialogFooter>
        </DialogContent>
      </Dialog>
    </div>
  )
}
